#include "circuit.h"

#include <gtest/gtest.h>

#include <cstring>
#include <new>
#include <set>
#include <string>

#include "circuit/instance.h"
#include "circuit/port.h"
#include "circuit/signal.h"
#include "vlsir/circuit.pb.h"

namespace bfg {
namespace {

std::set<std::string> PortNames(const vlsir::circuit::Module &module_pb) {
  std::set<std::string> names;
  for (const auto &port : module_pb.ports()) {
    names.insert(port.signal());
  }
  return names;
}

std::set<std::string> SignalNames(const vlsir::circuit::Module &module_pb) {
  std::set<std::string> names;
  for (const auto &signal : module_pb.signals()) {
    names.insert(signal.name());
  }
  return names;
}

TEST(CircuitTest, Signals) {
  Circuit circuit;
  circuit::Signal *a = circuit.AddSignal("a", 4);
  EXPECT_EQ("a", a->name());
  EXPECT_EQ(4, a->width());
  EXPECT_EQ(a, circuit.GetSignal("a"));
  EXPECT_EQ(nullptr, circuit.GetSignal("b"));
  EXPECT_EQ(a, circuit.GetOrAddSignal("a", 4));
  circuit::Signal *b = circuit.GetOrAddSignal("b", 1);
  EXPECT_EQ(b, circuit.GetSignal("b"));

  const Circuit &const_circuit = circuit;
  EXPECT_EQ(a, const_circuit.GetSignal("a"));
}

TEST(CircuitTest, AddSignal_DuplicateDies) {
  Circuit circuit;
  circuit.AddSignal("a", 1);
  EXPECT_DEATH(circuit.AddSignal("a", 1), "Duplicate signal name");
}

TEST(CircuitTest, PortsAreUniqueBySignal) {
  Circuit circuit;
  circuit::Signal *a = circuit.AddSignal("a", 1);
  circuit::Port *port = circuit.AddPort(*a, circuit::Port::INPUT);
  EXPECT_EQ(port, circuit.AddPort(*a, circuit::Port::OUTPUT));
  EXPECT_EQ(std::set<std::string>{"a"}, PortNames(circuit.ToVLSIRModule()));

  circuit.RemovePort("a");
  EXPECT_TRUE(PortNames(circuit.ToVLSIRModule()).empty());
}

TEST(CircuitTest, Instances) {
  Circuit child;
  Circuit circuit;
  circuit::Instance *instance = circuit.AddInstance("i", &child);
  EXPECT_EQ("i", instance->name());
  EXPECT_EQ(&child, instance->module());
  ASSERT_EQ(1, circuit.instances().size());
  EXPECT_DEATH(circuit.AddInstance("i", &child), "Duplicate instance name");
}

TEST(CircuitTest, ToVLSIRModule) {
  Circuit circuit;
  circuit.set_name("top");
  circuit::Signal *a = circuit.AddSignal("a", 1);
  circuit.AddSignal("internal", 1);
  circuit.AddPort(*a);
  Circuit child;
  child.set_name("child");
  circuit.AddInstance("i", &child);

  vlsir::circuit::Module module_pb = circuit.ToVLSIRModule();
  EXPECT_EQ("top", module_pb.name());
  EXPECT_EQ((std::set<std::string>{"a", "internal"}), SignalNames(module_pb));
  EXPECT_EQ(std::set<std::string>{"a"}, PortNames(module_pb));
  EXPECT_EQ(1, module_pb.instances_size());
}

TEST(CircuitTest, VLSIRModuleRoundTrip) {
  vlsir::circuit::Module module_pb;
  module_pb.set_name("m");
  auto *signal = module_pb.add_signals();
  signal->set_name("x");
  signal->set_width(2);
  module_pb.add_signals()->set_name("y");
  module_pb.add_ports()->set_signal("x");
  // Ports referring to unknown signals are skipped.
  module_pb.add_ports()->set_signal("nonexistent");

  std::unique_ptr<Circuit> circuit(Circuit::FromVLSIRModule(module_pb));
  EXPECT_EQ("m", circuit->name());
  ASSERT_NE(nullptr, circuit->GetSignal("x"));
  EXPECT_EQ(2, circuit->GetSignal("x")->width());

  vlsir::circuit::Module round_trip = circuit->ToVLSIRModule();
  EXPECT_EQ((std::set<std::string>{"x", "y"}), SignalNames(round_trip));
  EXPECT_EQ(std::set<std::string>{"x"}, PortNames(round_trip));
}

TEST(CircuitTest, VLSIRExternalModuleRoundTrip) {
  vlsir::circuit::ExternalModule module_pb;
  module_pb.mutable_name()->set_domain("lib");
  module_pb.mutable_name()->set_name("cell");
  module_pb.add_signals()->set_name("A");
  module_pb.add_ports()->set_signal("A");

  std::unique_ptr<Circuit> circuit(
      Circuit::FromVLSIRExternalModule(module_pb));
  EXPECT_EQ("lib", circuit->domain());
  EXPECT_EQ("cell", circuit->name());

  vlsir::circuit::ExternalModule round_trip = circuit->ToVLSIRExternalModule();
  EXPECT_EQ("lib", round_trip.name().domain());
  EXPECT_EQ("cell", round_trip.name().name());
  ASSERT_EQ(1, round_trip.ports_size());
  EXPECT_EQ("A", round_trip.ports(0).signal());
}

TEST(CircuitTest, AddCircuit) {
  Circuit other;
  circuit::Signal *x = other.AddSignal("x", 1);
  circuit::Signal *vdd = other.AddSignal("vdd", 1);
  other.AddGlobal(vdd);
  other.AddPort(*x);

  Circuit circuit;
  circuit.AddCircuit(other, "p");
  EXPECT_NE(nullptr, circuit.GetSignal("p__x"));
  // Globals are not prefixed.
  EXPECT_NE(nullptr, circuit.GetSignal("vdd"));
  EXPECT_EQ(nullptr, circuit.GetSignal("p__vdd"));
  EXPECT_EQ(std::set<std::string>{"p__x"}, PortNames(circuit.ToVLSIRModule()));
}

TEST(CircuitTest, Flatten) {
  Circuit child;
  child.set_name("child");
  circuit::Signal *in = child.AddSignal("in", 1);
  child.AddSignal("mid", 1);
  child.AddPort(*in);

  Circuit parent;
  circuit::Signal *net = parent.AddSignal("net", 1);
  circuit::Instance *instance = parent.AddInstance("i", &child);
  instance->Connect("in", *net);

  parent.Flatten();
  EXPECT_TRUE(parent.instances().empty());
  // The child's port signal is replaced by what it was connected to, and its
  // internal signals are prefixed with the instance name.
  EXPECT_NE(nullptr, parent.GetSignal("net"));
  EXPECT_NE(nullptr, parent.GetSignal("i__mid"));
  EXPECT_EQ(nullptr, parent.GetSignal("i__in"));
}

TEST(CircuitTest, Describe) {
  Circuit circuit;
  circuit.set_name("c");
  circuit.AddSignal("a", 1);
  EXPECT_NE(std::string::npos, circuit.Describe().find("1 signals"));
}

// Constructs T by default-initialisation over memory filled with a known
// pattern, so that uninitialised members show up deterministically.
template<typename T>
class OverGarbage {
 public:
  OverGarbage() {
    std::memset(storage_, 0xab, sizeof(storage_));
    object_ = new (storage_) T;
  }
  ~OverGarbage() { object_->~T(); }
  T *operator->() { return object_; }

 private:
  alignas(T) unsigned char storage_[sizeof(T)];
  T *object_;
};

TEST(CircuitTest, DefaultMembersAreInitialised) {
  OverGarbage<Circuit> circuit;
  EXPECT_EQ(nullptr, circuit->parent_cell());
  EXPECT_EQ("n_0", circuit->AddSignal("", 1)->name());
  EXPECT_EQ("n_1", circuit->AddSignal("", 1)->name());
}

}  // namespace
}  // namespace bfg
