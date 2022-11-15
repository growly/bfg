#include "poly_line_inflator.h"

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
#include "inflator_rules.pb.h"

namespace bfg {

using geometry::Line;
using geometry::LineSegment;
using geometry::PolyLine;
using geometry::Polygon;
using geometry::Point;
using geometry::Rectangle;

Layout *PolyLineInflator::Inflate(
    const RoutingGrid &routing_grid,
    const PolyLineCell &poly_line_cell) {
  std::unique_ptr<Layout> layout(new Layout(physical_db_));
  for (const auto &poly_line : poly_line_cell.poly_lines()) {
    LOG_IF(FATAL, !poly_line) << "poly_line is nullptr?!";

    Polygon polygon;
    LOG(INFO) << "inflating: " << poly_line->Describe();
    InflatePolyLine(*poly_line, &polygon);
    LOG(INFO) << " into: " << polygon.Describe();
    polygon.set_layer(poly_line->layer());

    auto bb = polygon.GetBoundingBox();
    layout->set_active_layer(poly_line->layer());
    layout->AddPolygon(polygon);
  }
  for (const auto &via : poly_line_cell.vias()) {
    Rectangle rectangle;
    InflateVia(
        routing_grid.GetRoutingViaInfoOrDie(
            via->bottom_layer(), via->top_layer()),
        *via,
        &rectangle);
    LOG(INFO) << "creating via on layer "
              << *physical_db_.GetLayerNameAndPurpose(rectangle.layer());
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
  LOG(INFO) << via;
  LOG_IF(FATAL, width == 0) << "Cannot create 0-width via.";
  LOG_IF(FATAL, height == 0) << "Cannot create 0-height via.";

  uint64_t half_width = width / 2;
  uint64_t half_height = height / 2;
  *rectangle = Rectangle(via.centre() - Point(half_width, half_height),
                         width,
                         height);
  rectangle->set_layer(layer);
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
void PolyLineInflator::InflatePolyLine(
    const PolyLine &polyline, Polygon *polygon) {
  LOG_IF(FATAL, polyline.segments().empty()) << "Inflating empty PolyLine";

  std::vector<Line> line_stack;
  std::unique_ptr<Line> last_shifted_line;

  // Since the PolyLine only stores the next point in each segment, we keep
  // track of the last one as we iterate through segments to create the lines
  // defined by (start, end) pairs.
  Point start = polyline.start();

  for (size_t i = 0; i < polyline.segments().size(); ++i) {
    const LineSegment &segment = polyline.segments().at(i);
    line_stack.emplace_back(start, segment.end);
    Line &line = line_stack.back();

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

    last_shifted_line = std::move(ShiftAndAppendIntersection(
          line, width, last_shifted_line.get(), polygon));
    start = segment.end;
  }
  polygon->AddVertex(last_shifted_line->end());

  last_shifted_line = nullptr;
  // TODO(aryap): lmao if you use size_t here it underflows and never exits
  // the loop.
  for (int i = line_stack.size() - 1; i >= 0; --i) {
    Line &line = line_stack.at(i);
    line.Reverse();

    if (i == line_stack.size() - 1) {
      polygon->AddVertex(line.start());
    }

    const LineSegment &segment = polyline.segments().at(i);
    double width = segment.width == 0 
        ? 100 : static_cast<double>(segment.width);

    last_shifted_line = std::move(ShiftAndAppendIntersection(
        line, width, last_shifted_line.get(), polygon));
  }
  // We flipped all the lines on the way back, so the last point is the 'end'
  // position of the first line in the list.
  polygon->AddVertex(last_shifted_line->end());
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

std::unique_ptr<Line> PolyLineInflator::ShiftAndAppendIntersection(
    const Line &next_source, double width, Line *last_shifted_line,
    Polygon *polygon) {

  std::unique_ptr<Line> shifted_line(GenerateShiftedLine(next_source, width));
  VLOG(3) << "Shifted " << next_source << " to " << *shifted_line;
  
  if (last_shifted_line == nullptr) {
    // Set the starting point.
    polygon->AddVertex(shifted_line->start());
    return shifted_line;
  }

  Point intersection;
  bool incident;
  if (Line::Intersect(
        *last_shifted_line, *shifted_line, &incident, &intersection)) {
    if (incident) {
      // Compute the midpoint of the two lines as the intersection. Because?
      const Line &lhs = *last_shifted_line;
      const Line &rhs = *shifted_line;
      double lhs_mid_y 
          = (lhs.end().y() - lhs.start().y())/2.0 + lhs.start().y();
      double rhs_mid_y 
          = (rhs.end().y() - rhs.start().y())/2.0 + rhs.start().y();
      double lhs_rhs_mid_y = (lhs_mid_y + rhs_mid_y) / 2.0;
      intersection = Point(lhs.start().x(), lhs_rhs_mid_y);
    }
    polygon->AddVertex(intersection);
  } else {
    // The lines never intersect, which means they're parallel. That also means
    // that we can simply insert the end of the last line and the start of the
    // next line as additional vertices to join the widths of the two:
    //
    //          v start
    //          +-----------+
    //          | next ^    |
    // ---------+           +----------
    // last ^   ^ end
    polygon->AddVertex(last_shifted_line->end());
    polygon->AddVertex(shifted_line->start());
  }
  
  // I'm expecting copy elision here.
  return shifted_line;
}

}  // namespace bfg
