#ifndef GEOMETRY_SHAPE_COLLECTION_H_
#define GEOMETRY_SHAPE_COLLECTION_H_

#include <map>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <optional>

#include "layer.h"
#include "polygon.h"
#include "port.h"
#include "rectangle.h"
#include "poly_line.h"
#include "../equivalent_nets.h"

#include "vlsir/layout/raw.pb.h"

namespace bfg {

class PhysicalPropertiesDatabase;

namespace geometry {

// A ShapeCollection contains a copy of some shapes, arranged according to
// their types. The ShapeCollection *owns* these shapes, which is why they are
// usually copies of something somewhere else.
class ShapeCollection : public Manipulable {
 public:
  ShapeCollection() = default;
  ShapeCollection(const ShapeCollection &other) {
    Add(other);
  }

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
  void AddConnectableShapesNotOnNets(
      const ShapeCollection &other, const EquivalentNets &nets);

  const Rectangle GetBoundingBox() const;

  bool Overlaps(const Rectangle &rectangle) const;

  void CopyConnectables(
      const std::optional<Layer> expected_layer,
      std::unordered_map<
          std::string,
          std::map<geometry::Layer,
                   std::unique_ptr<ShapeCollection>>> *shapes_by_layer_by_net)
      const;

  void RemoveNets(const EquivalentNets &nets);
  void KeepOnlyLayers(const std::set<geometry::Layer> &layers);

  ::vlsir::raw::LayerShapes ToVLSIRLayerShapes(
      const PhysicalPropertiesDatabase &db,
      bool include_non_pins = true,
      bool include_pins = true,
      size_t *count_out = nullptr) const;

  void PrefixNetNames(
      const std::string &prefix,
      const std::string &separator = ".");

  std::vector<std::unique_ptr<geometry::Rectangle>> &rectangles() {
    return rectangles_;
  }
  std::vector<std::unique_ptr<geometry::Polygon>> &polygons() {
    return polygons_;
  }
  std::vector<std::unique_ptr<geometry::Port>> &ports() {
    return ports_;
  }
  std::vector<std::unique_ptr<geometry::PolyLine>> &poly_lines() {
    return poly_lines_;
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
  const std::vector<std::unique_ptr<geometry::PolyLine>> &poly_lines() const {
    return poly_lines_;
  }

 private:
  void Add(const ShapeCollection &other,
           std::function<bool(const Rectangle&)> rectangle_filter,
           std::function<bool(const Polygon&)> polygon_filter,
           std::function<bool(const Port&)> port_filter,
           std::function<bool(const PolyLine&)> poly_line_filter);

  std::vector<std::unique_ptr<geometry::Rectangle>> rectangles_;
  std::vector<std::unique_ptr<geometry::Polygon>> polygons_;
  std::vector<std::unique_ptr<geometry::Port>> ports_;

  // TODO(aryap): This idea is half-baked.
  std::vector<std::unique_ptr<geometry::PolyLine>> poly_lines_;
};

}  // namespace geometry
}  // namespace bfg

#endif  // GEOMETRY_SHAPE_COLLECTION_H_
