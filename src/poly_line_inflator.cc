#include "poly_line_inflator.h"

#include <optional>
#include <glog/logging.h>
#include <cmath>
#include <memory>

#include "layout.h"
#include "poly_line_cell.h"
#include "routing_grid.h"
#include "geometry/point.h"
#include "geometry/line.h"
#include "geometry/poly_line.h"
#include "geometry/polygon.h"

namespace bfg {

using geometry::Line;
using geometry::LineSegment;
using geometry::PolyLine;
using geometry::Polygon;
using geometry::Point;
using geometry::Rectangle;

static bool IntersectsInBoundsAnyInRange(
    const Line &candidate,
    std::vector<Line>::const_reverse_iterator end,
    std::vector<Line>::const_reverse_iterator start) {
  for (std::vector<Line>::const_reverse_iterator it = end;
       it != start;
       it++) {
    bool incident = false;
    bool is_start_or_end = false;
    Point intersection;
    if (candidate.IntersectsInBounds(*it,
                                     &incident,
                                     &is_start_or_end,
                                     &intersection)) {
      return false;
    }
  }
  return false;
}

Layout *PolyLineInflator::Inflate(
    const RoutingGrid &routing_grid,
    const PolyLineCell &poly_line_cell) {
  std::unique_ptr<Layout> layout(new Layout(physical_db_));
  for (const auto &poly_line : poly_line_cell.poly_lines()) {
    LOG_IF(FATAL, !poly_line) << "poly_line is nullptr?!";

    std::optional<Polygon> polygon = InflatePolyLine(*poly_line);

    if (VLOG_IS_ON(12)) {
      LOG(INFO) << "inflating: " << poly_line->Describe() << " into: ";
      if (polygon) {
        LOG(INFO) << polygon->Describe();
      } else {
        LOG(INFO) << "(none)";
        continue;
      }
    }

    polygon->set_layer(poly_line->layer());

    auto bb = polygon->GetBoundingBox();
    layout->set_active_layer(poly_line->layer());
    layout->AddPolygon(*polygon);
  }
  for (const auto &via : poly_line_cell.vias()) {
    Rectangle rectangle;
    InflateVia(
        routing_grid.GetRoutingViaInfoOrDie(
            via->bottom_layer(), via->top_layer()),
        *via,
        &rectangle);
    layout->set_active_layer(rectangle.layer());
    layout->AddRectangle(rectangle);
  }
  return layout.release();
}

void PolyLineInflator::InflateVia(const RoutingViaInfo &info,
                                  const AbstractVia &via,
                                  Rectangle *rectangle) {
  InflateVia(info.layer, info.width, info.height, via, rectangle);
}

void PolyLineInflator::InflateVia(const geometry::Layer layer,
                                  int64_t width,
                                  int64_t height,
                                  const AbstractVia &via,
                                  Rectangle *rectangle) {
  LOG_IF(FATAL, width == 0) << "Cannot create 0-width via.";
  LOG_IF(FATAL, height == 0) << "Cannot create 0-height via.";

  uint64_t half_width = width / 2;
  uint64_t half_height = height / 2;
  *rectangle = Rectangle(via.centre() - Point(half_width, half_height),
                         width,
                         height);
  rectangle->set_layer(layer);
}

// An "inflated" Point is a bit strange. We only do this so we can provide a
// manipulated Polygon representation of the same point.
std::optional<Polygon> PolyLineInflator::InflatePoint(
    const Point &point, int64_t horizontal, int64_t vertical) {
  Point ll = point + Point(-horizontal, -vertical);
  Point ul = point + Point(-horizontal, vertical);
  Point ur = point + Point(horizontal, vertical);
  Point lr = point + Point(horizontal, -vertical);
  return Polygon({ll, ul, ur, lr});
}

// So, you could do this in one pass by inflating every central poly_line into
// its bounding lines, but that would create two problems when joining one
// segment to its immediate neighbour:
//                         1) deciding which two lines to intersect;
//                         2) finding their intersection.
//
// The inner and outer lines always intersect. 
//
// One way to determine which the "inner" and "outer" lines are is to
// bisect the angle ABC between the two joining segments AB &  BC, creating
// BD, then find the intersection E of the bisector BD with the line
// created by joining the distant ends of the joining segments intersection
// of the corner AC onto that line. The inner and outer lines can then be
// measured by measuring their projection from E onto the line defined by
// BD.
//
//     (A)
//     +
//     |\    (D)
//     | -  /
//     |  \/ (E)
//     |  /\
//     | /  -
//     |/    \
// (B) +------+ (C)
//    /
//   /
//
// The more naive (and simple) way seems to be to walk down the segments in one
// direction and then back in the other. This is still O(n). Treating them as
// vectors we can either keep track of the direction we're going in or reverse
// the start/end positions to reverse the vector itself. In either case, we
// take care to generate the shifted line in the same position relative to all
// vectors. sin/cos will do this for us if we compute the angle the vector
// makes to the positive x-axis correctly:
//                                   __
//                                  /  \
//        _ shifted vector      theta _|_______
//        /| _                      \/
//       /   /| original vector     /   / 
//      /   /                      /   /
//     /   /                      /   /
//    /   /                      /   /
//   /   /                      /   / shifted vector
//  /   /\ theta          +-> |/_  /
//     /_|_____           |      |/_ 
//                        |
//                        original vector, reversed
//
std::optional<Polygon> PolyLineInflator::InflatePolyLine(
    const PolyLine &polyline) {
  if (polyline.segments().empty()) {
    int64_t half_side = static_cast<int64_t>(
        std::max(polyline.overhang_start(), polyline.overhang_end()));
    LOG(WARNING) << "Inflating empty PolyLine as Point";
    return InflatePoint(polyline.start(), half_side, half_side);
  }
  Polygon polygon;
  // Carry over the net label.
  polygon.set_net(polyline.net());

  std::vector<Line> line_stack;
  std::vector<Line> forward_lines;
  std::vector<Line> reverse_lines;

  // Since the PolyLine only stores the next point in each segment, we keep
  // track of the last one as we iterate through segments to create the lines
  // defined by (start, end) pairs.
  Point start = polyline.start();

  // Generate shifted lines in the forward direction.
  for (size_t i = 0; i < polyline.segments().size(); ++i) {
    const LineSegment &segment = polyline.segments().at(i);
    line_stack.emplace_back(start, segment.end);
    // Take a copy so we can maybe stretch it.
    Line line = line_stack.back();

    // Stretch the start of the start, or end of the end segments according to
    // policy:
    if (i == 0 && polyline.overhang_start() > 0) {
      line.StretchStart(polyline.overhang_start());
    }
    if (i == polyline.segments().size() - 1 && polyline.overhang_end() > 0) {
      line.StretchEnd(polyline.overhang_end());
    }

    double width = segment.width == 0 ?
        100 : static_cast<double>(segment.width);

    std::unique_ptr<Line> shifted_line(GenerateShiftedLine(line, width));
    forward_lines.push_back(*shifted_line);

    start = segment.end;
  }

  // Repeat for the reverse direction.
  for (int i = line_stack.size() - 1; i >= 0; --i) {
    // Take a copy so we can reverse it!
    Line line = line_stack.at(i);
    line.Reverse();

    const LineSegment &segment = polyline.segments().at(i);
    double width = segment.width == 0 
        ? 100 : static_cast<double>(segment.width);

    std::unique_ptr<Line> shifted_line(GenerateShiftedLine(line, width));
    reverse_lines.push_back(*shifted_line);
  }

  AppendIntersections(forward_lines, &polygon);
  AppendIntersections(reverse_lines, &polygon);
 
  return polygon;
}

Line *PolyLineInflator::GenerateShiftedLine(
    const Line &source, double width,
    double extension_source, double extension_end) {
  // TODO(aryap): Integer division can lead to precision loss here,
  // so we *should* make sure we recover it.
  double half_width = static_cast<double>(width) / 2.0;

  double theta = source.AngleToHorizon();

  int64_t shift_x = static_cast<int64_t>(std::sin(theta) * half_width);
  int64_t shift_y = static_cast<int64_t>(std::cos(theta) * half_width);

  Line *shifted_line = new Line(source);
  shifted_line->Shift(-shift_x, shift_y);

  if (extension_source > 0.0) {
    int64_t extension_x =
        static_cast<int64_t>(std::cos(theta) * extension_source);
    int64_t extension_y =
        static_cast<int64_t>(std::sin(theta) * extension_source);
    shifted_line->ShiftStart(extension_x, extension_y);
  }

  if (extension_end > 0.0) {
    int64_t extension_x =
        static_cast<int64_t>(std::cos(theta) * extension_end);
    int64_t extension_y =
        static_cast<int64_t>(std::sin(theta) * extension_end);
    shifted_line->ShiftEnd(extension_x, extension_y);
  }

  return shifted_line;
}


// There is a very real problem when a line about-faces and goes back the
// way it came:
//
//
//                   |
//                   |
//                   |
//                   |
//                   |
//   (2)             v (1)
//   <------->-------
//        (3) |
//            |
//            |
//            |
//            v (4)
//
// This creates a loop, which makes sense since the lines go through 2*pi
// of turns, but it's not what we want.
//
// What's more, the treatment is different if the line (4) goes the other
// way:
//                   |
//        (4) ^      |
//            |      |
//            |      |
//            |      |
//   (2)      |      v (1)
//   <------->-------
//        (3)
//
// ... since now the loop is on the reverse side traversal, and the forward
// direction can proceed as normal. We also have to consider what happens if the
// line (2) is longer than the line (3).
//
//
// The treatment is different still if the about face appears on the other side
// of the first line (1), though that has symmetry to these cases but now for
// the reverse traversal.
//
// If we assume that about-faces don't happen one after the other (i.e. that if
// that happens the lines are simplified to redundant spans), then we should
// just be able to check if, immediately following an about-face, the shifted
// line we've generated intersects with any previously generated line, in
// bounds.
 
void PolyLineInflator::AppendIntersections(
    const std::vector<Line> &shifted_lines,
    Polygon *polygon) {
  if (shifted_lines.empty())
    return;

  // Always add the start vertex.
  polygon->AddVertex(shifted_lines.front().start());

  for (size_t i = 1; i < shifted_lines.size(); ++i) {
    const Line &current_line = shifted_lines.at(i);
    const Line &last_line = shifted_lines.at(i - 1);

    if (Line::AreAntiParallel(current_line, last_line)) {
      // Line turns 180 degrees to about-face. If this happens in the middle of
      // a line in one direction then the line has a kink in it and we have to
      //  - check for intersections of the current line with the line before
      //  last; 
      //  - check for inter
      //  - check for intersections of the _next line_ with the one before last
      //  (also in bounds).
      //
      // We have to assume that there are no back-to-back parallel or
      // anti-parallel lines to simplify this considerably. This invariant must
      // be enforced by the Line itself.
      if (i < shifted_lines.size() - 1) {
        const Line &next_line = shifted_lines.at(i + 1);

        bool incident = false;
        bool ignored = false;
        Point intersection;
        if (next_line.IntersectsInBounds(last_line,
                                         &incident,
                                         &ignored,
                                         &intersection)) {
          polygon->AddVertex(intersection);
          // Also skip the next line, since we've already effectively found the
          // intersection point it would yield.
          ++i;
          continue;
        }
      }

      if (i >= 2) {
        const Line &before_last_line = shifted_lines.at(i - 2);

        bool incident = false;
        Point intersection;
        if (before_last_line.IntersectsInMutualBounds(current_line,
                                                      &incident,
                                                      &intersection)) {
          // Have to replace the last polygon vertex with this intersection
          // instead:
          polygon->RemoveLastVertex();
          polygon->AddVertex(intersection);
          continue;
        }
      }
    }

    Point intersection;
    bool incident;
    if (Line::Intersect(last_line,
                        current_line,
                        &incident,
                        &intersection)) {
      if (!incident) {
        polygon->AddVertex(intersection);
      }
    } else {
      // The lines never intersect, which means they're parallel or
      // anti-parallel.  That also means that we can simply insert the end of
      // the last line and the start of the next line as additional vertices to
      // join the widths of the two:
      //
      //          v start
      //          +-----------+
      //            next ^
      // ---------+           +----------
      // last ^   ^ end
      //
      //
      //      last ---------+ < end
      //
      //      next ---------+ <start
      //
      polygon->AddVertex(last_line.end());
      polygon->AddVertex(current_line.start());
    }
  }

  // Always add the final vertex..
  polygon->AddVertex(shifted_lines.back().end());
}

}  // namespace bfg
