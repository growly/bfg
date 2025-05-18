#ifndef SCOPED_LAYER_H_
#define SCOPED_LAYER_H_

#include <string>

#include "geometry/layer.h"

namespace bfg {

class Layout;

// RAII. I can never remember why this is called "Resource Acquisition Is
// Initialization", but it is this pattern.
class ScopedLayer {
 public:
  ScopedLayer() = delete;

  ScopedLayer(Layout *layout, const geometry::Layer &layer);
  ScopedLayer(Layout *layout, const std::string &layer);
  ~ScopedLayer();

 private:
  Layout *layout_;
};

}  // namespace bfg

#endif  // SCOPED_LAYER_H_
