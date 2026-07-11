#ifndef CIRCUIT_CATENABLE_H_
#define CIRCUIT_CATENABLE_H_

#include "signal.h"
#include "slice.h"
#include "wire.h"
#include "concatenation.h"

namespace bfg {
namespace circuit {

class Concatenation;

// TODO(aryap): OOP says make Slices, Signals, Wires and other Concatenations
// implement some "Catenable" so we only need to store a simple pointer here
// and let dynamic dispatch do the rest. Unfortunately that is not a pattern
// I seem to have followed consistently.
//
// The real problem with doing that is that I don't know whether we'll need
// to recover the original type somewhere downstream (something we typically
// have needed), in which case all that niceness evaporates. So before we
// make a nice interface we have to figure out what all these Catenables have
// in common other than that they can go in this list. Probably something like:
//    - Get... names?
typedef std::variant<
    circuit::Wire,
    circuit::Slice,
    circuit::Signal*,
    circuit::Concatenation*> Catenable;

}  // namespace circuit
}  // namespace bfg

#endif  // CIRCUIT_CATENABLE_H_
