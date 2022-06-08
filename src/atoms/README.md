# Tiles & Atoms

> What's in a name?
> *Juliet*

A layout/circuit structure consistes of a hierarchy of `Tile`s, which
instantiate other `Tile`s and `Atom`s.

An `Atom` is the base generator for a given subcircuit in a compilation. `Atom`s
are parameterisable by semiconductor process parameters, usually with spacing
rules.

You should be able to ask an `Atom` to generate you a layout and a circuit
description given some parameters.

It is the job of a `Tile` to determine which `Atom`s to use and where. The
optionality in a `Tile` thus depends on the architecture and the process, but it
is not concerned with the details of a particular component. But the unit of
thing that gets swapped out is the `Atom`.
