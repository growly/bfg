#ifndef GEOMETRY_SHAPE_COLLECTION_H_
#define GEOMETRY_SHAPE_COLLECTION_H_

#include <sstream>

#include "polygon.h"
#include "port.h"
#include "rectangle.h"

namespace bfg {
namespace geometry {

class ShapeCollection : public Manipulable {
 public:
  std::string Describe() const;

  bool Empty() const;

  void MirrorY() override;
  void MirrorX() override;
  void Translate(const Point &offset) override;
  void ResetOrigin() override;
  void FlipHorizontal() override;
  void FlipVertical() override;
  void Rotate(int32_t degrees_ccw) override;

  void Add(const ShapeCollection &other);

  const Rectangle GetBoundingBox() const;

  std::vector<std::unique_ptr<geometry::Rectangle>> &rectangles() {
    return rectangles_;
  }
  std::vector<std::unique_ptr<geometry::Polygon>> &polygons() {
    return polygons_;
  }
  std::vector<std::unique_ptr<geometry::Port>> &ports() {
    return ports_;
  }

  const std::vector<std::unique_ptr<geometry::Rectangle>> &rectangles() const {
    return rectangles_;
  }
  const std::vector<std::unique_ptr<geometry::Polygon>> &polygons() const {
    return polygons_;
  }
  const std::vector<std::unique_ptr<geometry::Port>> &ports() const {
    return ports_;
  }

 private:
  std::vector<std::unique_ptr<geometry::Rectangle>> rectangles_;
  std::vector<std::unique_ptr<geometry::Polygon>> polygons_;
  std::vector<std::unique_ptr<geometry::Port>> ports_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_SHAPE_COLLECTION_H_
