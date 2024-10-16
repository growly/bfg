#include "port.h"

#include <absl/strings/str_join.h>

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "point.h"

namespace bfg {

namespace geometry {

bool Port::Compare(const Port &lhs, const Port &rhs) {
  return Point::CompareXThenY(lhs.lower_left_, rhs.lower_left_);
}

bool Port::Compare(
    const std::unique_ptr<Port> &lhs, const std::unique_ptr<Port> &rhs) {
  if (!lhs) return true;
  if (!rhs) return false;
  return Compare(*lhs, *rhs);
}

std::string Port::Describe() const {
  std::stringstream ss;
  ss << "[Port " << net_ << " " << lower_left_ << " " << upper_right_ << "]";
  return ss.str();
}

std::string Port::DescribePorts(const std::vector<geometry::Port*> &ports) {
  std::vector<std::string> port_descriptions;
  for (geometry::Port *port : ports) {
    port_descriptions.push_back(
        absl::StrFormat("(%d, %d)", port->centre().x(), port->centre().y()));
  }
  return absl::StrJoin(port_descriptions, ", ");
}

std::string Port::DescribePorts(const std::set<geometry::Port*> &ports) {
  std::vector<geometry::Port*> sorted_ports(ports.begin(), ports.end());
  std::sort(sorted_ports.begin(), sorted_ports.end(),
            [](geometry::Port *lhs, geometry::Port *rhs) {
              if (lhs->centre().x() == rhs->centre().x()) {
                return lhs->centre().y() < rhs->centre().y();
              }
              return lhs->centre().x() < rhs->centre().x();
            });
  return DescribePorts(sorted_ports);
}

}  // namespace geometry

std::ostream &operator<<(std::ostream &os, const geometry::Port &port) {
  return os << port.Describe();
}

}  // namespace bfg
