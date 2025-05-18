#include "scoped_layer.h"

#include <string>
#include <glog/logging.h>

#include "geometry/layer.h"
#include "layout.h"

namespace bfg {

ScopedLayer::ScopedLayer(Layout *layout, const geometry::Layer &layer)
    : layout_(layout) {
  layout->set_active_layer(layer);
}

ScopedLayer::ScopedLayer(Layout *layout, const std::string &layer)
    : layout_(layout) {
  layout->SetActiveLayerByName(layer);
}

ScopedLayer::~ScopedLayer() {
  layout_->RestoreLastActiveLayer();
}


}  // namespace bfg
