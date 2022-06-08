#ifndef CIRCUIT_CONNECTION_H_
#define CIRCUIT_CONNECTION_H_

namespace bfg {
namespace circuit {

enum ConnectionType {
  SIGNAL,
  SLICE,
  CONCATENATION
}

class Connection {
 private:
  Connection() = default;

 public:
  ConnectionType connection_type_;

  Signal *signal_;
  // Slice *slice_;
  // Concatenation *concatenation_;
};

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_CONNECTION_H_
