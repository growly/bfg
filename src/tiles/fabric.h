#ifndef TILES_FABRIC_H_
#define TILES_FABRIC_H_

#include "tile.h"

#include "proto/parameters/fabric.pb.h"

namespace bfg {

class DesignDatabase;

namespace tiles {

class Fabric : public Tile {
 public:
  struct Parameters {
    void ToProto(proto::parameters::Fabric *pb) const {}
    void FromProto(const proto::parameters::Fabric &pb) {}
  };

  Fabric(const Parameters &parameters, DesignDatabase *design_db)
      : Tile(design_db),
        parameters_(parameters) {}

  Cell *Generate() override;

 private:
  Parameters parameters_;
};

}  // namespace tiles
}  // namespace bfg

#endif  // TILES_FABRIC_H_
