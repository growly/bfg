#ifndef CELL_REFERENCE_H_
#define CELL_REFERENCE_H_

#include <memory>
#include <string>

#include "vlsir/utils.pb.h"

namespace bfg {

// References to cells by name. INTERNAL-type references only use a name.
// EXTERNAL-type references use a name and a domain. An un-set reference has
// type NONE.
class CellReference {
 public:
  enum Type {
    NONE,
    INTERNAL,
    EXTERNAL
  };

  static CellReference FromVLSIRReference(
      const vlsir::utils::Reference &reference_pb) {
    switch (reference_pb.to_case()) {
      case vlsir::utils::Reference::ToCase::kExternal:
        return CellReference(
            reference_pb.external().domain(), reference_pb.external().name());
      default:
        return CellReference(reference_pb.local());
    }
  }

  // This is all the default initialisation, but explicitly note that default
  // CellReferences are set to type NONE.
  CellReference()
      : type_(NONE) {}

  CellReference(const std::string &domain, const std::string &name)
      : type_(EXTERNAL), domain_(domain), name_(name) {}

  CellReference(const std::string &name)
      : type_(INTERNAL), domain_(""), name_(name) {}

  const vlsir::utils::Reference ToVLSIRReference() const {
    vlsir::utils::Reference reference_pb;
    switch (type_) {
      case EXTERNAL:
        reference_pb.mutable_external()->set_domain(domain_);
        reference_pb.mutable_external()->set_name(name_);
      default:
        reference_pb.set_local(name_);
    }
    return reference_pb;
  }

  const Type &type() const { return type_; }
  const std::string &domain() const { return domain_; }
  const std::string &name() const { return name_; }

 private:
  Type type_;

  std::string domain_;
  std::string name_;
};

}  // namespace bfg

#endif  // CELL_REFERENCE_H_

