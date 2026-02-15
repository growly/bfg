#include "rounded_rectangle.h"

#include <vector>
#include <cmath>
#include <glog/logging.h>
#include <sstream>

#include "arc.h"
#include "point.h"
#include "../physical_properties_database.h"

namespace bfg {
namespace geometry {

// Tests if the given point is within the corner_radius_ of the given centre
// point.
bool RoundedRectangle::WithinRadius(
    const Point &centre,
    const Point &test_point) const {
  return centre.L2DistanceTo(test_point) <= static_cast<double>(
      std::abs(corner_radius_));
}

std::tuple<Point, Point, Point, Point> RoundedRectangle::GetInnerCoordinates()
    const {
  Point inner_lower_left = lower_left_ + Point{corner_radius_, corner_radius_};
  Point inner_upper_left = UpperLeft() + Point{corner_radius_, -corner_radius_};
  Point inner_upper_right = upper_right_ + Point{
      -corner_radius_, -corner_radius_};
  Point inner_lower_right = LowerRight() + Point{-corner_radius_, corner_radius_};
  return {inner_lower_left,
          inner_upper_left,
          inner_upper_right,
          inner_lower_right};
}

RoundedRectangle::Regions RoundedRectangle::GetRegions() const {
  auto [inner_lower_left,
        inner_upper_left,
        inner_upper_right,
        inner_lower_right] = GetInnerCoordinates();

  Regions regions;
  regions.centre = Rectangle(inner_lower_left, inner_upper_right);
  regions.left = Rectangle(inner_lower_left + Point{-corner_radius_, 0},
                           inner_upper_right);
  regions.upper = Rectangle(inner_upper_left,
                            inner_upper_right + Point{0, corner_radius_});
  regions.right = Rectangle(inner_lower_right,
                            inner_upper_right + Point{corner_radius_, 0});
  regions.lower = Rectangle(inner_lower_left - Point{0, corner_radius_},
                            inner_lower_right);
  regions.upper_left = Rectangle(inner_upper_left + Point{-corner_radius_, 0},
                                 inner_upper_left + Point{0, corner_radius_});
  regions.upper_right = Rectangle(inner_upper_right, upper_right_);
  regions.lower_right = Rectangle(inner_lower_right + Point(0, -corner_radius_),
                                  inner_lower_right + Point(corner_radius_, 0));
  regions.lower_left = Rectangle(lower_left_, inner_lower_left);

  regions.lower_left_arc = Arc(inner_lower_left, corner_radius_, 180, 270);
  regions.upper_left_arc = Arc(inner_upper_left, corner_radius_, 90, 180);
  regions.upper_right_arc = Arc(inner_upper_right, corner_radius_, 0, 90);
  regions.lower_right_arc = Arc(inner_lower_right, corner_radius_, 270, 0);

  return regions;
}

// Check for overlap in each of the 5 inner rectangles, since these are the
// easy cases:
//     \  +-----------------------+
//     r\ |r     UPPER            |
//       \|                       |
// +------+-----------------------+------+
// | LEFT |    width - 2r         |  r   |
// |      |                height |      |
// |  r   |     CENTRE       - 2r |RIGHT |
// +------+-----------------------+------+
//       /|     LOWER             |
//     r/ |                       |
//     L  +-----------------------+
bool RoundedRectangle::Overlaps(const Rectangle &other) const {
  if (!GetBoundingBox().Overlaps(other.GetBoundingBox()))
    return false;

  auto [inner_lower_left,
        inner_upper_left,
        inner_upper_right,
        inner_lower_right] = GetInnerCoordinates();

  Rectangle centre_region(inner_lower_left, inner_upper_right);
  if (centre_region.Overlaps(other))
    return true;

  Rectangle left_region(inner_lower_left + Point{-corner_radius_, 0},
                        inner_upper_right);
  if (left_region.Overlaps(other))
    return true;

  Rectangle upper_region(inner_upper_left,
                         inner_upper_right + Point{0, corner_radius_});
  if (upper_region.Overlaps(other))
    return true;

  Rectangle right_region(inner_lower_right,
                         inner_upper_right + Point{corner_radius_, 0});
  if (right_region.Overlaps(other))
    return true;

  Rectangle lower_region(inner_lower_left - Point{0, corner_radius_},
                         inner_lower_right);
  if (lower_region.Overlaps(other))
    return true;

  Rectangle upper_left_region(inner_upper_left + Point{-corner_radius_, 0},
                              inner_upper_left + Point{0, corner_radius_});
  if (upper_left_region.Overlaps(other)) {
    return WithinRadius(inner_upper_left, other.LowerRight());
  }

  Rectangle upper_right_region(inner_upper_right, upper_right_);
  if (upper_right_region.Overlaps(other)) {
    return WithinRadius(inner_upper_right, other.lower_left());
  }

  Rectangle lower_right_region(inner_lower_right + Point(0, -corner_radius_),
                               inner_lower_right + Point(corner_radius_, 0));
  if (lower_right_region.Overlaps(other)) {
    return WithinRadius(inner_upper_right, other.UpperLeft());
  }

  Rectangle lower_left_region(lower_left_, inner_lower_left);
  if (lower_left_region.Overlaps(other)) {
    return WithinRadius(inner_lower_left, other.upper_right());
  }

  LOG(FATAL) << "Could not find overlap. But how?!";
  return false;
}

bool RoundedRectangle::Overlaps(const RoundedRectangle &other) const {
  if (!GetBoundingBox().Overlaps(other.GetBoundingBox()))
    return false;

  Regions our_regions = GetRegions();
  Regions their_regions = other.GetRegions();

  std::vector<Rectangle*> ours_easy = {
      &our_regions.centre,
      &our_regions.left,
      &our_regions.upper,
      &our_regions.right,
      &our_regions.lower};
  std::vector<Rectangle*> theirs_easy = {
      &their_regions.centre,
      &their_regions.left,
      &their_regions.upper,
      &their_regions.right,
      &their_regions.lower};

  for (Rectangle *ours : ours_easy) {
    for (Rectangle *theirs : theirs_easy) {
      if (ours->Overlaps(*theirs)) {
        return true;
      }
    }
  }

  // If none of the inner regions overlap, it must be an overlap between the
  // corner regions.
  std::vector<std::pair<Point, Point>> comparisons = {
    {our_regions.lower_left.upper_right(),
      their_regions.upper_right.lower_left()},
    {our_regions.upper_left.LowerRight(),
      their_regions.lower_right.UpperLeft()},
    {our_regions.upper_right.lower_left(),
      their_regions.lower_left.upper_right()},
    {our_regions.lower_right.UpperLeft(),
      their_regions.upper_left.LowerRight()}
  };
  for (const auto &pair : comparisons) {
    double distance = pair.first.L2DistanceTo(pair.second);
    if (distance <= static_cast<double>(
          corner_radius_ + other.corner_radius_)) {
      return true;
    }
  }

  return false;
}

const std::string RoundedRectangle::Describe() const {
  std::stringstream ss;
  ss << "[RoundedRectangle " << lower_left_ << " " << upper_right_
     << " cr: " << corner_radius_ << "]";
  return ss.str();
}

}   // namespace geometry

std::ostream &operator<<(
    std::ostream &os, const geometry::RoundedRectangle &rounded_rectangle) {
  os << rounded_rectangle.Describe();
  return os;
}

}   // namespace bfg
