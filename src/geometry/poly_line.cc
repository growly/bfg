#include "poly_line.h"

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <glog/logging.h>
#include <sstream>
#include <vector>
#include <optional>

#include <absl/cleanup/cleanup.h>

#include "line_segment.h"
#include "point.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

std::string PolyLine::Describe() const {
  std::stringstream ss;
  ss << "<" << overhang_start_ << "| " << start_;
  for (const auto &segment : segments_) {
    ss << " |" << segment.width << "| " << segment.end;
  }
  ss << " |" << overhang_end_ << ">";
  if (net_ != "") {
    ss << " net: " << net_;
  }
  return ss.str();
}

void PolyLine::MirrorY() {
  start_.set_x(-start_.x());
  for (LineSegment &segment : segments_) {
    segment.end.set_x(-segment.end.x());
  }
}

void PolyLine::MirrorX() {
  start_.set_y(-start_.y());
  for (LineSegment &segment : segments_) {
    segment.end.set_y(-segment.end.y());
  }
}

void PolyLine::FlipHorizontal() {
  Rectangle bounding_box = GetBoundingBox();
  int64_t left_x = bounding_box.lower_left().x();
  int64_t right_x = bounding_box.upper_right().x();
  start_.set_x(right_x - (start_.x() - left_x));
  for (LineSegment &segment : segments_) {
    segment.end.set_x(right_x - (segment.end.x() - left_x));
  }
}

void PolyLine::FlipVertical() {
  Rectangle bounding_box = GetBoundingBox();
  int64_t upper_y = bounding_box.upper_right().y();
  int64_t lower_y = bounding_box.lower_left().y();
  start_.set_y(upper_y - (start_.y() - lower_y));
  for (LineSegment &segment : segments_) {
    segment.end.set_y(upper_y - (segment.end.y() - lower_y));
  }
}

void PolyLine::Translate(const Point &offset) {
  start_ += offset;
  for (LineSegment &segment : segments_) {
    segment.end += offset;
  }
}

const Rectangle PolyLine::GetBoundingBox() const {
  int64_t min_x = start_.x();
  int64_t max_x = start_.x();
  int64_t min_y = start_.y();
  int64_t max_y = start_.y();

  for (const auto &segment : segments_) {
    min_x = std::min(segment.end.x(), min_x);
    min_y = std::min(segment.end.y(), min_y);
    max_x = std::max(segment.end.x(), max_x);
    max_y = std::max(segment.end.y(), max_y);
  }

  Rectangle bounding_box = Rectangle(Point(min_x, min_y), Point(max_x, max_y));
  bounding_box.set_layer(layer_);
  return bounding_box;
};

void PolyLine::AddSegment(const Point &to, const uint64_t width) {
  const Point &last = segments_.empty() ? start_ : segments_.back().end;
  // Skip duplicate segments.
  if (to == last) {
    LOG(WARNING) << "Skipping duplicate PolyLine segment to " << to;
    return;
  }
  if (to.x() != last.x() && to.y() != last.y()) {
    LOG(FATAL) << "PolyLine segments must be rectilinear. Make sure the new "
               << "x == last_x or y == last_y.";
  }
  segments_.push_back(LineSegment{to, width});
}

void PolyLine::InsertForwardBulgePoint2(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
    size_t intersection_index, const Line &intersected_line) {
  // The starting point for the line we've intersected.
  Line current_line = intersected_line;
  double current_length = point.L2DistanceTo(segments_[intersection_index].end);

  double on_axis_overflow = static_cast<double>(coaxial_length) / 2.0;
  double off_axis_overflow = static_cast<double>(coaxial_width) / 2.0;

  std::optional<uint64_t> previous_segment_original_width = std::nullopt;

  size_t k = intersection_index;
  Point reference_point = point;
  do {
    LineSegment &current_segment = segments_[k];

    uint64_t current_segment_original_width = current_segment.width;

    // The original intersected line gives the axis of the bulge, so we measure
    // relative to that:
    double theta = current_line.AngleToLineCounterClockwise(intersected_line);
    double sin_theta = std::sin(theta);
    double cos_theta = std::cos(theta);

    // The length required of this segment to "escape" the bulge.
    double required_length = std::round(
        std::abs(on_axis_overflow * cos_theta) +
        std::abs(off_axis_overflow * sin_theta));
    double half_required_width =
        std::abs(on_axis_overflow * sin_theta) +
        std::abs(off_axis_overflow * cos_theta);

    uint64_t new_width = static_cast<uint64_t>(
        std::round(2.0 * half_required_width));

    // If current_length exceeds required_length, the current segments takes us
    // out of the bulge, and we're done.
    VLOG(13) << current_line
             << " [fwd] k: " << k
             << " current_length: " << current_length
             << " required_length: " << required_length
             << " reference_point: " << reference_point
             << " theta: " << theta
             << " on_axis: " << on_axis_overflow
             << " off_axis: " << off_axis_overflow;

    // This is a very annoying special case to avoid creating notches and it
    // doesn't make a lot of sense. The projection of the width of the previous
    // segment must be accommodated when changing direction. Otherwise, by
    // creating a segment only long enough to escape the bulge, we might be
    // creating smaller segments that contradict the thicker segments originally
    // around them. See the InsertBulge_DoesNotCreateNotch test.
    if (previous_segment_original_width) {
      LineSegment &previous_segment = segments_[k - 1];
      Line previous_line = Line(previous_segment.end, current_segment.end);
      double alpha = previous_line.AngleToLineCounterClockwise(current_line);
      double sin_alpha = std::sin(alpha);

      double old_required_length = required_length;

      double half_previous_segment_width = 
          static_cast<double>(*previous_segment_original_width) / 2.0;
      double previous_width_projection =
          std::round(half_previous_segment_width);
      if (sin_alpha != 0) {
        previous_width_projection = 
            std::round(std::abs(half_previous_segment_width / sin_alpha));
      }
      required_length = std::max(required_length, previous_width_projection);
      LOG_IF(INFO, previous_width_projection != (*previous_segment_original_width / 2.0)) << "previous segment width: "
                << (*previous_segment_original_width / 2.0)
                << " previous_width_projection: " << previous_width_projection
                << " alpha: " << alpha
                << " sin_alpha: " << sin_alpha
                << " required_length: " << required_length
                << " vs prev. " << old_required_length;
    }

    if (current_length > required_length) {
      // Only insert a new segment if the existing width was exceeded.
      if (new_width > current_segment_original_width) {
        Point new_boundary = current_line.PointOnLineAtDistance(
            reference_point, required_length);

        segments_.insert(segments_.begin() + k,
                         LineSegment {
                           .end = new_boundary,
                           .width = std::max(current_segment.width, new_width)
                         });
      }
      break;
    }

    current_segment.width = std::max(current_segment.width, new_width);

    if (current_length == required_length) {
      break;
    }

    // Updates:
    on_axis_overflow -= std::abs(current_length * cos_theta);
    off_axis_overflow -= std::abs(current_length * sin_theta);

    // TODO(aryap): This does not consider overhang_end_. The easiest way to do
    // that is to treat the end point of the final segment as being pushed out
    // by overhang_end_ distance along the final segment. Likewise for the
    // start.
    if (k == segments_.size() - 1) {
      // We hit the last segment without getting out of the bulge. Elongate the
      // segment by shifting the final end point. But, if the final segment was
      // thicker than the bulge, we have to add a new segment and continue only
      // the with required thickness.
      Point new_boundary = current_line.PointOnLineAtDistance(
          reference_point, required_length);

      if (enable_narrowing_extensions_ &&
          current_segment_original_width > new_width) {
        segments_.push_back(LineSegment {
            .end = new_boundary,
            .width = new_width });
      } else {
        segments_[k].end = new_boundary;
      }
      break;
    }

    reference_point = segments_[k].end;
    current_line = Line(segments_[k].end, segments_[k + 1].end);
    current_length = current_line.Length();
    previous_segment_original_width = current_segment_original_width;

    ++k;
  } while (k < segments_.size());
}

void PolyLine::InsertForwardBulgePoint(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
    size_t intersection_index, const Line &intersected_line) {

  double half_length = static_cast<double>(coaxial_length) / 2.0;

  // - Proceed to the end of all segments in a straight line, stopping at the
  // first turn or when the bulge end point would fall on the current
  // segment (index k).
  // - Fatten covered segments in the line to at least the width of the bulge.
  // - When the loop terminates, either we are at the end of the line, the next
  // segment turns a corner, or we have to insert a point into the current
  // segment.

  // Just after the first corner turn or at the end of the poly line we have to
  // insert a new point to maintain the bulge shape when the poly line is
  // inflated.

  double d_end = point.L2DistanceTo(segments_[intersection_index].end);
  double d_insertion = half_length;
  double overflow = d_insertion - d_end;

  Point insertion_start = point;

  size_t k = intersection_index;
  while (k < segments_.size() - 1) {
    Line next_line = Line(segments_[k].end, segments_[k + 1].end);

    if (!intersected_line.IsSameInfiniteLine(next_line))
      // Next segment turns a corner, so break.
      break;

    if (overflow <= 0) {
      // Inserted point should fall on the current segment (k), so break.
      break;
    }

    insertion_start = segments_[k].end;
    double next_line_length = next_line.Length();
    d_insertion = overflow;
    overflow -= next_line_length;

    // The current segment (k) is exceeded, so it must be at least as wide as
    // the new bulge. But as a cleaner alternative, we just delete the existing
    // segment. We can do this because the next segment is on the same line and
    // would become the same width.
    if (segments_[k].width <= coaxial_width) {
      if (segments_[k + 1].width <= coaxial_width) {
        segments_.erase(segments_.begin() + k);
        continue;
      }
      segments_[k].width = coaxial_width;
    }

    ++k;
  }

  uint64_t half_width = std::llround(static_cast<double>(
        std::max(segments_[k].width, coaxial_width) / 2.0));

  // TODO(aryap): Close to zero good enough here?
  if (overflow < 0) {
    // The segment to alter is k, but only if it would increase the segment's
    // existing width:
    if (segments_[k].width <= coaxial_width) {
      Point point_after = intersected_line.PointOnLineAtDistance(
          insertion_start, d_insertion);
      segments_.insert(segments_.begin() + k,
                       LineSegment {
                         .end = point_after,
                         .width = coaxial_width,
                       });
    }
  } else if (overflow == 0 || k == segments_.size() - 1) {
    // Effectively at the end of the poly line or only need to change the width
    // of this line, so only need modify the last segment instead of inserting
    // a new one.
    if (segments_[k].width <= coaxial_width) {
      segments_[k].end = intersected_line.PointOnLineAtDistance(
          segments_[k].end, overflow);
      segments_[k].width = coaxial_width;
    } else if (overflow > 0) {
      Point point_after =
          intersected_line.PointOnLineAtDistance(segments_[k].end, overflow);
      segments_.push_back(
          LineSegment { .end = point_after, .width = coaxial_width });
    }
  } else {
    // The next segment is around a corner.
    //
    // The intersected segment `segment` will remain and be enlarged to the
    // given width.
    segments_[k].width = std::max(coaxial_width, segments_[k].width);

    // A new segment should created along the next segment we were going to
    // embark on, with an appropriate new width.
    LineSegment &next_segment = segments_[k + 1];
    Line current_line = Line(
        k == 0 ? start_ : segments_[k - 1].end, segments_[k].end);
    Line next_line = Line(segments_[k].end, next_segment.end);

    // Let theta be the angle from the next line to the previous. Then the
    // distance along the axis of the next segment to the next segment end point
    // is half_width / sin(theta). The width of that segment must be at least
    // overflow * sin(theta).
    //
    // The proof is as follows: trust me bro
    //
    // (Also note that since sin(theta) = -sin(2*pi - theta) we can simplify
    // life by just taking the absolute value and not worrying about the
    // relative angle.)
    double theta = next_line.AngleToLineCounterClockwise(current_line);
    double sin_theta = std::sin(theta);

    // Minimum length of the new segment, found by project the width of this
    // bulge onto the diverging line:
    double minimum_length = sin_theta == 0 ?
        overflow : std::abs(half_width / sin_theta);

    // Minimum width of new segment to cover far corner of the bulge we are
    // trying insert:
    uint64_t overflow_width = static_cast<uint64_t>(2.0 * std::abs(
        overflow * sin_theta + half_width * std::cos(theta)));

    double width_consumed = std::abs(next_line.Length() * sin_theta);
    uint64_t remaining_width = static_cast<uint64_t>(
        std::max(0.0, 2.0 * (half_width - width_consumed)));

    Point point_after = next_line.PointOnLineAtDistance(
        segments_[k].end, minimum_length);

    if (next_line.IntersectsInBounds(point_after)) {
      segments_.insert(
          segments_.begin() + k + 1,
          LineSegment {
            .end = point_after,
            .width = std::max(overflow_width, next_segment.width)
      });
    } else {
      // As a special case, we must consider if the next point on the line of
      // the next segment exceeds that segment's length! If it does, just give
      // up and inflate the width of the next segment to subsume our overflow
      // length.
      next_segment.width = std::max(overflow_width, next_segment.width);

      if (k + 2 < segments_.size()) {
        segments_[k + 2].width = std::max(
            remaining_width, segments_[k + 2].width);
      } else {
        // next_segment is the last segment, so we should just extend it.
        next_segment.end = point_after;
      }
    }
  }
}

// For this and InsertForwardBulgePoint, the general problem to solve is that of
// modifying PolyLine segments so that, in the resulting polygon, a rectangular
// bulge of at least the given width and length is covered around the given
// point `point`.
//
// To do this most generally, we need to follow line segments out from the
// bulge centre point until we find one that lands outside. We have to modify
// the widths (and sometimes lengths) of segments to ensure the bulge appears
// correctly.
void PolyLine::InsertBackwardBulgePoint2(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
    size_t intersection_index, const Line &intersected_line,
    uint64_t intersected_previous_width) {

  // The starting point for the line we've intersected.
  const Point &line_start =
      intersection_index == 0 ? start_ : segments_[intersection_index - 1].end;

  Line current_line = intersected_line;
  double current_length = point.L2DistanceTo(line_start);

  double on_axis_overflow = static_cast<double>(coaxial_length) / 2.0;
  double off_axis_overflow = static_cast<double>(coaxial_width) / 2.0;

  std::optional<uint64_t> previous_segment_original_width = std::nullopt;

  size_t k = intersection_index;
  Point reference_point = point;
  do {
    LineSegment &current_segment = segments_[k];

    // Because InsertBackwardsBulgePoint comes after InsertForwardsBulgePoint,
    // it's possible that the intersected segment's width was already changed
    // in the forward pass, so we must use the original value passed in. That
    // can only happen to the intersected segment though.
    uint64_t current_segment_original_width = k == intersection_index ?
        intersected_previous_width : current_segment.width;

    // The original intersected line gives the axis of the bulge, so we measure
    // relative to that:
    double theta = current_line.AngleToLineCounterClockwise(intersected_line);
    double sin_theta = std::sin(theta);
    double cos_theta = std::cos(theta);

    double required_length = std::round(
        std::abs(on_axis_overflow * cos_theta) +
        std::abs(off_axis_overflow * sin_theta));
    double half_required_width =
        std::abs(on_axis_overflow * sin_theta) +
        std::abs(off_axis_overflow * cos_theta);

    uint64_t new_width =
        static_cast<uint64_t>(std::round(2.0 * half_required_width));

    // If current_length exceeds required_length, the current segments takes us
    // out of the bulge, and we're done.
    VLOG(13) << current_line
             << " [rwd] k: " << k
             << " current_length: " << current_length
             << " required_length: " << required_length
             << " reference_point: " << reference_point
             << " theta: " << theta
             << " on_axis: " << on_axis_overflow
             << " off_axis: " << off_axis_overflow;

    current_segment.width = std::max(current_segment.width, new_width);

    if (previous_segment_original_width) {
      LineSegment &previous_segment = segments_[k + 1];
      Line previous_line = Line(current_segment.end, previous_segment.end);
      double alpha = previous_line.AngleToLineCounterClockwise(current_line);
      double sin_alpha = std::sin(alpha);

      double old_required_length = required_length;

      double half_previous_segment_width = 
          static_cast<double>(*previous_segment_original_width) / 2.0;
      double previous_width_projection =
          std::round(half_previous_segment_width);
      if (sin_alpha != 0) {
        previous_width_projection = 
            std::round(std::abs(half_previous_segment_width / sin_alpha));
      }
      // FIXME: why does this work unless a change is made for sin_alpha == 0?
      required_length = std::max(required_length, previous_width_projection);
      LOG_IF(INFO, previous_width_projection != (*previous_segment_original_width / 2.0)) << "previous segment width: "
                << (*previous_segment_original_width / 2.0)
                << " previous_width_projection: " << previous_width_projection
                << " alpha: " << alpha
                << " sin_alpha: " << sin_alpha
                << " required_length: " << required_length
                << " vs prev. " << old_required_length;
    }

    if (current_length > required_length) {

      // Only insert a new segment if the existing width was exceeded.
      if (new_width > current_segment_original_width) {
        Point new_boundary = current_line.PointOnLineAtDistance(
            reference_point, -required_length);

        // NOTE(aryap): We use a negative distance because we are traversing
        // lines backwards.
        segments_.insert(segments_.begin() + k,
                         LineSegment {
                           .end = new_boundary,
                           .width = current_segment_original_width
                         });
      }
      break;
    } else if (current_length == required_length) {
      break;
    }

    // Updates:
    on_axis_overflow -= std::abs(current_length * cos_theta);
    off_axis_overflow -= std::abs(current_length * sin_theta);

    if (k == 0) {
      // We hit the last segment without getting out of the bulge. Elongate the
      // segment by shifting the start_ point. Widen the segement to the minimum
      // required.
      Point new_boundary = current_line.PointOnLineAtDistance(
          reference_point, -required_length);

      if (enable_narrowing_extensions_ &&
          current_segment_original_width > new_width) {
        segments_.insert(segments_.begin(),
                         LineSegment {
                             .end = start_,
                             .width = new_width });
      }
      start_ = new_boundary;
      break;
    }

    reference_point = segments_[k - 1].end;
    current_line = Line(
        k == 1 ? start_ : segments_[k - 2].end,
        segments_[k - 1].end);
    current_length = current_line.Length();
    previous_segment_original_width = current_segment_original_width;

    --k;
  } while (k >= 0);
}

void PolyLine::InsertBackwardBulgePoint(
    const Point &point, uint64_t coaxial_width, uint64_t coaxial_length,
    size_t intersection_index, const Line &intersected_line,
    uint64_t intersected_previous_width) {

  const Point &start =
      intersection_index == 0 ? start_ : segments_[intersection_index - 1].end;

  double half_length = static_cast<double>(coaxial_length) / 2.0;

  double d_start = point.L2DistanceTo(start);
  double d_insertion = half_length;
  double overflow = d_insertion - d_start;

  size_t k = intersection_index;
  while (k > 0) {
    Line previous_line = Line(
        k == 1 ? start_ : segments_[k - 2].end,
        segments_[k - 1].end);

    if (!intersected_line.IsSameInfiniteLine(previous_line)) {
      break;
    }

    if (overflow <= 0) {
      break;
    }

    double previous_line_length = previous_line.Length();
    d_insertion = overflow;
    overflow -= previous_line_length;

    --k;

    if (segments_[k].width <= coaxial_width) {
      if (segments_[k + 1].width <= coaxial_width) {
        segments_.erase(segments_.begin() + k);
      } else {
        segments_[k].width = coaxial_width;
      }
    }
  }

  uint64_t half_width = std::llround(static_cast<double>(
        std::max(segments_[k].width, coaxial_width) / 2.0));

  if (overflow < 0) {
    if (segments_[k].width <= coaxial_width) {
      Point point_before = intersected_line.PointOnLineAtDistance(
          point, -d_insertion);
      segments_.insert(segments_.begin() + k,
                       LineSegment {
                         .end = point_before,
                         .width = (k == intersection_index ?
                            intersected_previous_width : segments_[k].width)
                       });
    }
  } else if (overflow == 0) {
    segments_[k].width = std::max(segments_[k].width, coaxial_width);
  } else if (k == 0) {
    start_ = intersected_line.PointOnLineAtDistance(start_, -overflow);
  } else {
    // Skip inflating segment k width since it should be done in the forward
    // pass.

    // In this case we have overflow and a corner turn.
    LineSegment &last_segment = segments_[k - 1];
    Line current_line = Line(last_segment.end, point);

    uint64_t previous_width = last_segment.width;
    Point last_line_start = k == 1 ?  start_ : segments_[k - 2].end;
    // The end of the last line is the start of this line. The start of the
    // last line is the end of the line before it.
    Line last_line = Line(last_line_start, last_segment.end);

    double theta = last_line.AngleToLineCounterClockwise(current_line);
    double sin_theta = std::sin(theta);
    double minimum_length = sin_theta == 0 ?
        overflow : std::abs(half_width / sin_theta);
    uint64_t overflow_width = static_cast<uint64_t>(2.0 * std::abs(
        overflow * sin_theta + half_width * std::cos(theta)));

    double width_consumed = std::abs(last_line.Length() * sin_theta);
    uint64_t remaining_width = static_cast<uint64_t>(
        std::max(0.0, 2.0 * (half_width - width_consumed)));

    Point point_before = last_line.PointOnLineAtDistance(
        last_segment.end, -minimum_length);

    last_segment.width = std::max(overflow_width, previous_width);
    if (last_line.IntersectsInBounds(point_before)) {
      segments_.insert(
          segments_.begin() + k - 1,
          LineSegment {
            .end = point_before,
            .width = previous_width
          });
    } else {
      if (k - 1 > 0) {
        segments_[k - 2].width = std::max(
            remaining_width, segments_[k - 2].width);
      } else {
        start_ = point_before;
      }
    }
  }
}


// FIXME(aryap): REMOVE:
void PolyLine::InsertBulge(
    const Point point, uint64_t coaxial_width, uint64_t coaxial_length) {
  size_t intersection_index = 0;
  if (!Intersects(point, &intersection_index)) {
    return;
  }

  // We have to take an explicit copy of the point we're given, because it's
  // possible (i.e. it happened once) that the reference be to some value in
  // this line that we're about to modify. E.g. if you
  // poly_line.InsertBulge(poly_line.End()), you will modify the underlying
  // value half way.

  // TODO(aryap): This doesn't quite work yet, so I'm leaving these here:
  // LOG(INFO) << Describe();
  // LOG(INFO) << "point = " << point << " w x l " << coaxial_width << " x " << coaxial_length;

  const Point &start =
      intersection_index == 0 ? start_ : segments_[intersection_index - 1].end;
  Line line = Line(start, segments_[intersection_index].end);

  // InsertForwardBulgePoint might modify this so we save the previous width of
  // the segment at intersection_index.
  uint64_t previous_width = segments_[intersection_index].width;

  InsertForwardBulgePoint2(
      point, coaxial_width, coaxial_length, intersection_index, line);
  // LOG(INFO) << "after forwards (" << coaxial_width << ", " << coaxial_length
  //           << "): " << Describe();

  InsertBackwardBulgePoint2(
      point, coaxial_width, coaxial_length, intersection_index, line,
      previous_width);
  // LOG(INFO) << "after backwards (" << coaxial_width << ", " << coaxial_length
  //           << "): " << Describe();

  EnforceInvariants();
}


//           _
//           /|
//          o <- want this point after
//         /
//        x
//       /
//      o <- want this point before
//     /
void PolyLine::InsertBulge2(
    const Point point, uint64_t coaxial_width, uint64_t coaxial_length) {
  size_t intersection_index = 0;
  if (!Intersects(point, &intersection_index)) {
    return;
  }

  // We have to take an explicit copy of the point we're given, because it's
  // possible (i.e. it happened once) that the reference be to some value in
  // this line that we're about to modify. E.g. if you
  // poly_line.InsertBulge(poly_line.End()), you will modify the underlying
  // value half way.

  // TODO(aryap): This doesn't quite work yet, so I'm leaving these here:
  // LOG(INFO) << Describe();
  // LOG(INFO) << "point = " << point << " w x l " << coaxial_width << " x " << coaxial_length;

  const Point &start =
      intersection_index == 0 ? start_ : segments_[intersection_index - 1].end;
  Line line = Line(start, segments_[intersection_index].end);

  // InsertForwardBulgePoint might modify this so we save the previous width of
  // the segment at intersection_index.
  uint64_t previous_width = segments_[intersection_index].width;

  InsertForwardBulgePoint(
      point, coaxial_width, coaxial_length, intersection_index, line);
  //LOG(INFO) << "after forwards (" << coaxial_width << ", " << coaxial_length
  //          << "): " << Describe();

  InsertBackwardBulgePoint(
      point, coaxial_width, coaxial_length, intersection_index, line,
      previous_width);
  //LOG(INFO) << "after backwards (" << coaxial_width << ", " << coaxial_length
  //          << "): " << Describe();

  EnforceInvariants();
}

void PolyLine::InsertBulgeLater(
    const Point point, uint64_t coaxial_width, uint64_t coaxial_length) {
  deferred_bulges_.push_back(
      DeferredBulge {point, coaxial_width, coaxial_length});
}

void PolyLine::ApplyDeferredBulges() {
  for (const DeferredBulge &deferred : deferred_bulges_) {
    InsertBulge(deferred.position, deferred.width, deferred.length);
  }
  deferred_bulges_.clear();
}

void PolyLine::SetWidth(const uint64_t width) {
  for (LineSegment &segment : segments_) {
    segment.width = width;
  }
}

const std::vector<Point> PolyLine::Vertices() const {
  std::vector<Point> points = {start_};
  for (const auto &segment : segments_) {
    points.push_back(segment.end);
  }
  return points;
}

bool PolyLine::Intersects(const Point &point, size_t *segment_index) const {
  Point start = start_;
  int64_t k = -1;
  for (size_t i = 0; i < segments_.size(); ++i) {
    const LineSegment &segment = segments_[i];
    Line line = Line(start, segment.end);
    if (line.IntersectsInBounds(point)) {
      k = i;
      break;
    }
    start = segment.end;
  }
  if (segment_index)
    *segment_index = k;
  return k >= 0;
}

void PolyLine::EnforceInvariants() {
  // Remove duplicate segment end-points, enlarging the remaining to the max of
  // all the segment widths which had the same end-point.
  Point last = start_;
  auto last_segment = segments_.end();
  for (auto it = segments_.begin(); it != segments_.end();) {
    if (it->end == last) {
      if (last_segment != segments_.end()) {
        last_segment->width = std::max(last_segment->width, it->width);
      }
      it = segments_.erase(it);
    } else {
      last = it->end;
      last_segment = it;
      ++it;
    }
  }

  // FIXME(aryap): This should definitely be its own funtion.
  // Remove successive segments in a line with the same width.
  if (!segments_.empty()) {
    Line previous_line = Line(start_, segments_[0].end);
    for (auto it = segments_.begin() + 1; it != segments_.end();) {
      LineSegment &previous_segment = *(it - 1);
      LineSegment &current_segment = *it;
      Line current_line = Line(previous_segment.end, current_segment.end);
      absl::Cleanup rotate_line = [&]() {
        previous_line = current_line;
      };
      if (!current_line.IsSameInfiniteLine(previous_line) ||
          current_segment.width != previous_segment.width) {
        ++it;
        continue;
      }
      previous_segment.end = current_segment.end;
      it = segments_.erase(it);
    }
  }


  // TODO(aryap): Remove anti-parallel segments that overlap parallel
  // segments...?

  // If min_separation_ is defined, also remove segments in a straight line
  // that would a violation of min_separation between the segments before and
  // after due to contracting/expanding widths:
  //
  //       > min_separation_
  //       <--->
  //  ----+     +----
  //      |     |
  //      +-----+
  //
  //      +-----+
  //      |     |
  //  ----+     +----
  //
  if (!min_separation_)
    return;
  for (size_t i = 1; i < segments_.size() - 1; ++i) {
    const LineSegment &last_segment = segments_[i - 1];
    const LineSegment &this_segment = segments_[i];
    const LineSegment &next_segment = segments_[i + 1];

    Line last_line(i == 1 ? start_ : segments_[i - 2].end, last_segment.end);
    Line this_line(last_segment.end, this_segment.end);
    Line next_line(this_segment.end, next_segment.end);

    if (!last_line.IsSameInfiniteLine(this_line) ||
        !next_line.IsSameInfiniteLine(this_line))
      continue;

    int64_t length = static_cast<uint64_t>(this_line.Length());

    if (this_segment.width < last_segment.width &&
        this_segment.width < next_segment.width &&
        length < *min_separation_) {
      segments_.erase(segments_.begin() + i);
      --i;
    }
  }
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os,
                         const geometry::PolyLine &poly_line) {
  os << poly_line.Describe();
  return os;
}

}  // namespace bfg
