# Tiles & Atoms

> What's in a name?
> *Juliet*

A layout/circuit structure comprises a hierarchy of `Tile`s, which instantiate
other `Tile`s and `Atom`s.

An `Atom` is the base generator for a given subcircuit in a compilation. `Atom`s
are parameterisable by semiconductor process parameters, usually with spacing
rules. `Atom`s understand how to make a basic digital circuit element, like a
latch, or a flip-flop, or a multiplexer.  You should be able to ask an `Atom` to
generate you a layout and a circuit description given some parameters.

It is the job of a `Tile` to determine which `Atom`s to use and where. The
optionality in a `Tile` thus depends on the architecture and the process, but it
is not concerned with the details of a particular component. `Tile`s do need to
worry about how to piece and route other components together, but not much more
than that.

## Sky130TransmissionGate

![A few different design points.](/assets/img/transmission_gate.gif)

## Sky130TransmissionGateStack

![A few different design points.](/assets/img/transmission_gate_stack.gif)
