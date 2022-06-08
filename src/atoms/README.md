# Atoms

A layout/circuit structure consistes of a hierarchy of `Tile`s, which
instantiate other `Tile`s and `Atom`s.

An `Atom` is the base generator for a given subcircuit in a compilation. `Atom`s
are parameterisable by semiconductor process parameters, usually with spacing
rules.
