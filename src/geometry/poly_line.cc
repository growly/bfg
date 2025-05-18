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
#include "radian.h"
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

void PolyLine::ExtendToInclude(const Point &point) {
  // Check if the point lands within any segment.
  if (LineAtPoint(point)) {
    return;
  }
  Line begin_line = Line(start_, segments_.front().end);
  Line end_line = Line(
      segments_.size() > 1 ? (segments_.end() - 1)->end : start_,
      segments_.back().end);
  std::vector<Line> lines = {begin_line, end_line};
  Point nearest_begin = begin_line.PointOnLineClosestTo(point);
  Point nearest_end = end_line.PointOnLineClosestTo(point);
  bool intersects_begin = begin_line.Intersects(point);
  bool intersects_end = end_line.Intersects(point);
  if (!intersects_begin && !intersects_end) {
    // Do nothing.
    return;
  }
  if (intersects_begin && !intersects_end) {
    start_ = point;
    return;
  }
  if (!intersects_begin && intersects_end) {
    segments_.back().end = point;
    return;
  }
  // Intersects both.
  if (nearest_begin.L2DistanceTo(point) < nearest_end.L2DistanceTo(point)) {
    start_ = point;
  } else {
    segments_.back().end = point;
  }
}

std::optional<Line> PolyLine::LineAtPoint(const Point &point) const {
  for (size_t i = 0; i < segments_.size(); ++i) {
    const LineSegment &segment = segments_.at(i);
    Line line = Line(
        i == 0 ? start_ : segments_.at(i - 1).end,
        segments_.at(i).end);
    if (line.IntersectsInBounds(point)) {
      return line;
    }
  }
  return std::nullopt;
}

double PolyLine::ComputeRequiredLengthForLastSegmentWidth(
    uint64_t previous_segment_original_width,
    const Line &current_line,
    const Line &previous_line,
    double required_length) {
  double alpha = previous_line.AngleToLineCounterClockwise(current_line);
  double sin_alpha = std::sin(alpha);

  // Sometimes we get a number very close to zero, but no actually zero!
  // Like 1.2246467991473532e-16 D-:
  if (Radian::IsEffectivelyZero(sin_alpha)) {
    // No change.
    return required_length;
  }

  double half_previous_segment_width =
      static_cast<double>(previous_segment_original_width) / 2.0;
  double previous_width_projection =
      std::round(std::abs(half_previous_segment_width / sin_alpha));
  return std::max(required_length, previous_width_projection);
}

std::tuple<double, uint64_t, double, double>
    PolyLine::ComputeRequiredLengthAndWidth(
        const Line &intersected_line,
        const Line &current_line,
        double on_axis_overflow,
        double off_axis_overflow) {
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

    return std::make_tuple(
        required_length, new_width, sin_theta, cos_theta);
}

void PolyLine::InsertForwardBulgePoint(
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

    double required_length;
    uint64_t new_width;
    double sin_theta;
    double cos_theta;
    std::tie(required_length, new_width, sin_theta, cos_theta) =
        ComputeRequiredLengthAndWidth(intersected_line,
                                      current_line,
                                      on_axis_overflow,
                                      off_axis_overflow);

    // If current_length exceeds required_length, the current segments takes us
    // out of the bulge, and we're done.
    VLOG(13) << current_line
             << " [fwd] k: " << k
             << " current_length: " << current_length
             << " required_length: " << required_length
             << " reference_point: " << reference_point
             << " on_axis: " << on_axis_overflow
             << " off_axis: " << off_axis_overflow;

    // This is a very annoying special case to avoid creating notches and it
    // doesn't make a lot of sense. The projection of the width of the previous
    // segment must be accommodated when changing direction. Otherwise, by
    // creating a segment only long enough to escape the bulge, we might be
    // creating smaller segments that contradict the thicker segments originally
    // around them. See the InsertBulge_DoesNotCreateNotch test.
    if (previous_segment_original_width) {
      LOG_IF(FATAL, k == 0)
          << "Assume k > 0 here because previous_segment_original_width not "
          << "set until k incremented";
      Line previous_line = Line(
          k > 1 ? segments_[k - 2].end : start_, segments_[k - 1].end);

      required_length = ComputeRequiredLengthForLastSegmentWidth(
          previous_segment_original_width.value(),
          current_line,
          previous_line,
          required_length);
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
    // by overhang_end_ distance along the final segment. Likewise for
    // overhang_start_ at the start.
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

// For this and InsertForwardBulgePoint, the general problem to solve is that of
// modifying PolyLine segments so that, in the resulting polygon, a rectangular
// bulge of at least the given width and length is covered around the given
// point `point`.
//
// To do this most generally, we need to follow line segments out from the
// bulge centre point until we find one that lands outside. We have to modify
// the widths (and sometimes lengths) of segments to ensure the bulge appears
// correctly.
void PolyLine::InsertBackwardBulgePoint(
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

    double required_length;
    uint64_t new_width;
    double sin_theta;
    double cos_theta;
    std::tie(required_length, new_width, sin_theta, cos_theta) =
        ComputeRequiredLengthAndWidth(intersected_line,
                                      current_line,
                                      on_axis_overflow,
                                      off_axis_overflow);

    // If current_length exceeds required_length, the current segments takes us
    // out of the bulge, and we're done.
    VLOG(13) << current_line
             << " [rwd] k: " << k
             << " current_length: " << current_length
             << " required_length: " << required_length
             << " reference_point: " << reference_point
             << " on_axis: " << on_axis_overflow
             << " off_axis: " << off_axis_overflow;

    current_segment.width = std::max(current_segment.width, new_width);

    if (previous_segment_original_width) {
      LOG_IF(FATAL, k == segments_.size() - 1)
          << "Assume k < final here because previous_segment_original_width "
          << "not set until k incremented";
      Line previous_line = Line(segments_[k].end, segments_[k + 1].end);

      required_length = ComputeRequiredLengthForLastSegmentWidth(
          previous_segment_original_width.value(),
          current_line,
          previous_line,
          required_length);
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

void PolyLine::InsertBulge(const Point &point,
                           uint64_t width,
                           uint64_t length,
                           std::optional<double> angle_rads) {
  if (!angle_rads) {
    InsertBulgeInternal(point, width, length);
    return;
  }

  // Project width of box at `angle_rads` onto the coaxial width of the line,
  // and respectively the length onto the coaxial length.
  auto line = LineAtPoint(point);
  if (!line) {
    return;
  }

  double alpha = line->AngleToHorizon() - *angle_rads;

  double coaxial_width = static_cast<double>(width) * std::cos(alpha) +
                         static_cast<double>(length) * std::sin(alpha);
  double coaxial_length = static_cast<double>(length) * std::cos(alpha) + 
                          static_cast<double>(width) * std::sin(alpha);

  InsertBulgeInternal(point,
                      std::llround(std::abs(coaxial_width)),
                      std::llround(std::abs(coaxial_length)));
}

//           _
//           /|
//          o <- want this point after
//         /
//        x
//       /
//      o <- want this point before
//     /
void PolyLine::InsertBulgeInternal(
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
    const Point point,
    uint64_t coaxial_width,
    uint64_t coaxial_length,
    std::optional<double> angle_rads) {
  deferred_bulges_.push_back(
      DeferredBulge {point, coaxial_width, coaxial_length, angle_rads});
}

void PolyLine::ApplyDeferredBulges() {
  for (const DeferredBulge &deferred : deferred_bulges_) {
    if (deferred.angle_rads) {
      InsertBulge(deferred.position,
                  deferred.width,
                  deferred.length,
                  deferred.angle_rads);
    } else {
      InsertBulge(deferred.position, deferred.width, deferred.length);
    }
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

void PolyLine::RemoveNotchesAroundCorners() {
  //
  //             /        /
  //            /        /
  //           /        /
  //          /        / ^
  //         +        +  |
  //         |        |  | does this notch violate min_separation_?
  //         |        |  |
  //         |        |  v
  //    +----+        +----+
  //    |                  |
  //    |                  |
  //    |                  |

  // In the most general case, we have to check all pairs of segments that are
  // not adjacent to see if they violate min_separation_. This can occur if the
  // two segments are in a straight line and the maximum width of any segment
  // between them is less than both of their widths, _or_ if of the othe
  // segments is at a non-zero angle to the other. One way of doing this is to
  // track the change in spacing with each successive segment given its angle to
  // previous segments and the original, for each original in the list of
  // segments. Another way is to generate the boundary lines and find the
  // distance between them, basically pre-empting the inflation process itself.
  //
  // Ok the principle of this algorithm is as follows:
  //  - find the axis between the end of a starting line and the start of
  //  another line (at least 2 segments away)
  //  - find the angles of the starting and subsequent lines to the intervening
  //  axis
  //  - project the widths of lines at each end onto the intervening axis,
  //  these are the quantities by which the space between the lines is reduced
  //  in the worst case
  //
  // TODO(aryap): Currently use std::abs to avoid this problem, but is treatment
  // of various angles around the unit circle (where sin becomes negative)
  // correct?
  if (!min_separation_)
    return;
  for (size_t i = 0; i < segments_.size() - 1; ++i) {
    LineSegment &first_segment = segments_[i];
    Line first_line = Line(i == 0 ? start_ : segments_[i - 1].end,
                           first_segment.end);

    if (i + 1 >= segments_.size()) {
      break;
    }
    // This is half-baked idea to shortcut work when there are no intervening
    // segments with smaller widths (as would be required for a notch in some
    // cases).
    // if (segments_[i + 1].width >= first_segment.width) {
    //   continue;
    // }

    uint64_t intervening_width = segments_[i + 1].width;

    for (size_t j = i + 2; j < segments_.size(); ++j) {
      LineSegment &last_segment = segments_[j - 1];
      LineSegment &next_segment = segments_[j];

      intervening_width = last_segment.width;

      Line next_line = Line(segments_[j - 1].end, next_segment.end);
      Line intervening_line = Line(first_line.end(), next_line.start());

      double theta_first =
          intervening_line.AngleToLineCounterClockwise(first_line);
      double theta_next =
          next_line.AngleToLineCounterClockwise(intervening_line);

      double intervening_length = intervening_line.Length();

      double first_projection_onto_intervening =
          std::abs(std::sin(theta_first) *
              static_cast<double>(first_segment.width) / 2.0);
      double next_projection_onto_intervening =
          std::abs(std::sin(theta_next) *
              static_cast<double>(next_segment.width) / 2.0);

      double spacing = intervening_length - (
          first_projection_onto_intervening + next_projection_onto_intervening);

      VLOG(16) << "i: " << i << ", " << "j: " << j << std::endl
               << "first_segment: " << first_segment.end << std::endl
               << "last_segment: " << last_segment.end << std::endl
               << "next_segment: " << next_segment.end << std::endl
               << "first_line: " << first_line << std::endl
               << "intervening_line: " << intervening_line << std::endl
               << "next_line: " << next_line << std::endl
               << "theta_first: " << theta_first << std::endl
               << "theta_next: " << theta_next << std::endl
               << "intervening_length: " << intervening_length << std::endl
               << "first_projection_onto_intervening: "
               << first_projection_onto_intervening << std::endl
               << "next_projection_onto_intervening: "
               << next_projection_onto_intervening << std::endl
               << "spacing: " << spacing;
      if (spacing >= *min_separation_) {
        // Nothing to do.
        continue;
      }

      // Widen all intervening segments to the width of the max within the span:
      std::vector<uint64_t> span_widths;
      // We measure the maximum width of segments in the span [i, j], i.e.
      // inclusive of i and j.
      std::transform(segments_.begin() + i,
                     segments_.begin() + j + 1,
                     std::back_inserter(span_widths),
                     [](const LineSegment &segment) { return segment.width; });
      uint64_t max_width_in_span =
          *std::max_element(span_widths.begin(), span_widths.end());
      // But we overwrite the widths of only the segments interior to the span,
      // excluding i and j themselves, (i, j).
      for (size_t k = i + 1; k < j; ++k) {
        segments_[k].width = max_width_in_span;
      }
    }
  }
}

void PolyLine::RemoveNotchesInAStraightLine() {
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
      // TODO(aryap): If instead of erasing this segment we set its with to the
      // min (or max) of its neighbours, we will remove the notch. The
      // RemoveRedundantSegments() process should then simplify the duplicate
      // segments down to one. However when I tried this it yielded weird
      // behaviour around a turn, which it shouln't? Needs testing.
      segments_.erase(segments_.begin() + i);
      --i;
    }
  }
}

void PolyLine::ReplaceDuplicateEndPointsWithWidest() {
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
}

void PolyLine::RemoveRedundantSegments() {
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
}

void PolyLine::EnforceInvariants() {
  ReplaceDuplicateEndPointsWithWidest();
  RemoveRedundantSegments();
  RemoveNotchesInAStraightLine();
  RemoveNotchesAroundCorners();
  // TODO(aryap): Remove anti-parallel segments that overlap parallel
  // segments...?
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os,
                         const geometry::PolyLine &poly_line) {
  os << poly_line.Describe();
  return os;
}

}  // namespace bfg
