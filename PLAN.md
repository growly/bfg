# Test coverage plan

Baseline (2026-11-21, gcov, `bfg_test` only): 18.8% of lines (4207/22430) and
34.7% of functions (644/1858) in `src/` are exercised by the 419 existing tests.

Status key: `[ ]` todo, `[x]` done, `[~]` in progress.

## Phase 0: make coverage measurable (done 2026-11-22)

- [x] `BFG_COVERAGE` CMake option adding `--coverage`
- [x] `scripts/coverage.sh` builds, runs and summarises (via
      `scripts/coverage_summary.py`, which can also list unexecuted
      functions per file)
- [ ] Optional: report coverage in CI (skipped for now: CI builds in Docker
      without the option)

## Phase 1: pure-logic units (done 2026-11-21)

- [x] `geometry/compass.cc`: proto <-> enum round-trip, `CompassHasNorth`,
      `CompassHasSouth`, `operator<<` (new `geometry/compass_test.cc`)
- [x] `parameter.cc`: `FromInteger`, SI prefix mapping, `FromVLSIRParameter`,
      `ToVLSIRParameter` (new `parameter_test.cc`)
- [x] `equivalent_nets.cc`: `ContainsAny`, `Add`, `Delete`, `Describe`
      (extended `equivalent_nets_test.cc`)
- [x] `geometry/shape_collection.cc`: `KeepOnlyNets`, `RemoveNets`,
      `KeepOnlyLayers`, `PrefixNetNames`, `Consume`, `Add*` variants,
      `GetBoundingBox`, `Describe` (new `geometry/shape_collection_test.cc`)
- [x] `geometry/poly_line.cc`: `ExtendToInclude`, `GetBoundingBox`,
      `LineAtPoint`, `FlipHorizontal`, `FlipVertical`, deferred bulges
      (extended `geometry/poly_line_test.cc`)
- [x] `geometry/rectangle.cc`: `AccumulatedOver`, `WithKeepout` (extended
      `geometry/rectangle_test.cc`)

| File | Lines before | Lines after | Functions before | Functions after |
|---|---|---|---|---|
| `geometry/compass.cc` | 0% | 97% | 0/5 | 5/5 |
| `parameter.cc` | 0% | 82% | 0/5 | 5/5 |
| `equivalent_nets.cc` | 35% | 85% | 3/8 | 7/8 |
| `geometry/shape_collection.cc` | 5% | 59% | 1/44 | 32/44 |
| `geometry/poly_line.cc` | 58% | 79% | 18/32 | 28/32 |
| `geometry/rectangle.cc` | 68% | 73% | 22/34 | 24/34 |

Overall: 18.8% -> 20.9% of lines, 34.7% -> 38.2% of functions. 479 tests pass.

## Phase 2: layout and instance transforms (done 2026-11-21)

- [x] `layout.cc`: `Translate`, `Rotate`, `MirrorX`, `MirrorY` (shapes,
      points and instances), `ResetOrigin`, `Flatten`, `ConsumeLayout`,
      `AddLayout` (new `layout_test.cc`)
- [x] `layout.cc`: `MakeVia`, `StampVias`, `MakePin`, `MakeWire`,
      `MakeAlternatingWire`, `MakeVerticalSpineWithFingers`,
      `ResolveEdgeSpec`, `Copy*Shapes*`, ports, `GetInstancesByName`,
      `EraseLayer`, `RemoveLayerFromChildTemplates`
- [x] `geometry/instance.cc`: `GeneratePorts`, `GetNearestPortNamed`,
      `GetFurthestPortNamed`, `GetMidwayPortNamed`, `GetFirstPortNamed`,
      `Copy*Shapes*`, `GetPoint`, `GetTilingBounds`, `MoveTilingLowerLeft`
      (extended `geometry/instance_test.cc`)
- [x] `geometry/port.cc` (new `geometry/port_test.cc`)
- [ ] Still untested: `Layout::ToVLSIRLayout`, `Describe`, `MakeViaEncap`,
      `DistributeVias`, `FlipHorizontal`/`FlipVertical` (layout, instance and
      shape collection)

| File | Lines before | Lines after | Functions before | Functions after |
|---|---|---|---|---|
| `layout.cc` | 5% | 65% | 4/68 | 45/68 |
| `geometry/instance.cc` | 4% | 83% | 1/36 | 28/36 |
| `geometry/port.cc` | 0% | 97% | 0/9 | 9/9 |
| `geometry/shape_collection.cc` | 59% | 67% | 32/44 | 36/44 |

Overall: 20.9% -> 24.8% of lines, 38.2% -> 44.7% of functions. 549 tests pass.

## Phase 3: routing

- [ ] New `routing_grid_test.cc`: `ShortestPath` (open and blocked),
      `AddRouteBetween`, `AddRouteToNet`, `AddMultiPointRoute`,
      `VerticesAreTooCloseForVias` at the spacing boundary, `FindViaStack`,
      `LayersReachableByVia`, temporary blockage setup/teardown
- [ ] New `routing_path_test.cc`: `Cost`, `SpannedVertices*`, `Legalise`,
      `AbbreviateOnce`, `CheckStillAvailable` (characterise first; see commit
      c3f6292), `CheckForNotchesToPerpendicularEdges` near min-spacing
- [ ] `route_manager_test.cc`: `Solve`, `Connect`, `ConnectMultiplePorts`,
      `ConnectToNet`
- [ ] `routing_track_test.cc`: `MarkEdgeAsUsed`, `AddTemporaryBlockage`,
      `RemoveTemporaryBlockage`, `CreateNewVertexAndConnect`, `VerticesInSpan`
- [ ] `routing_blockage_cache_test.cc`: `IsEdgeBlocked`, `BlockagesMatching`,
      `ValidAgainstKnownBlockages`
- [ ] `physical_properties_database`: `FindViaStackImpl`,
      `FindLayersReachableThroughOneVia`, `TypicalViaEncap`,
      `GetAccessibleLayersForPin`

## Phase 4: generator smoke tests

For each atom/tile, `Generate()` with default and edge parameters, then check
the cell has layout and circuit, expected ports exist, circuit ports match
layout pins, the bounding box is sane, and device counts match parameters.

- [ ] Atoms: `sky130_mux`, `sky130_split_buffer`, `sky130_interconnect_mux1`,
      `sky130_interconnect_mux2`, `sky130_transmission_gate`,
      `sky130_transmission_gate_stack`, `sky130_dfxtp`, `sky130_decap`,
      `sky130_carry1`, `sky130_buf`, `sky130_hd_mux2_1`, `sky130_xor2`,
      `sky130_tap`, `sky130_switch_complex`, `sky130_passive_mux3`,
      `gf180mcu_mux`
- [ ] Tiles: `lut_b`, `lut`, `s44`, `reduced_tile`, `interconnect`,
      `interconnect_wire_block`, `slice`, `fabric`
- [ ] Fill in or delete the empty `Sky130TransmissionGateTest.ViaLocations`
- [ ] Revisit the 3 disabled tests

## Phase 5: placement helpers and databases

- [ ] `row_guide.cc`
- [ ] `memory_bank.cc`
- [ ] `checkerboard_guide.cc`
- [ ] `circuit.cc`, `design_database.cc`

## Bugs found by new tests

All fixed on branch `test-coverage`, with their tests enabled:

- [x] `PolyLine(const PolyLine&)` did not copy the `Shape` base: copies lost
      layer, net and `is_connectable`. Affected every `ShapeCollection` copy.
      (c3b8d66)
- [x] `Parameter::FromVLSIRParameter` left `unit_prefix` uninitialised for
      unprefixed values. (77963ee)
- [x] `PolyLine::ExtendToInclude` built the last line from the last vertex to
      itself, and with one segment always moved the end. (71ea093)
- [x] `EquivalentNets::Delete` of the primary left `primary_` naming a net not
      in the set; now the first remaining name is promoted. (ce84157)
- [x] `operator<<(Compass)` printed `LOWER:` and `LOWER_LEFT:`. (4a79a6d)
- [x] `ShapeCollection::GetBoundingBox` threw `bad_optional_access` with no
      rectangles, polygons or ports, which `Layout::GetBoundingBox` would hit
      for any such layer. (f02da50)

## Bugs found in phase 2

Fixed on branch `test-coverage`, with their tests enabled:

- [x] `Layout::DeletePorts` (and `Sky130InterconnectMux1`) default-constructed
      a `PortSet`, whose comparator is a null function pointer: crash on the
      second port. (dd83de7)
- [x] `Layout::ResolveEdgeSpec` wrote every match to `*from` and could only
      match both endpoints on one instance. No callers yet. (14149f4)
- [x] `ShapeCollection::Consume` left null entries in the source. (4493487)
- [x] `Layout::ConsumeLayout` did not index consumed ports by net. (3fe04e6)
- [x] `Layout::MirrorX` did not mirror instance positions. (9a1e261)
- [x] `Instance::GetMidwayPortNamed` ignored its name and used `"D"`.
      (b390991)

## Open questions (need a decision before fixing)

Instance rotation and reflection are applied inconsistently. Disabled tests in
`geometry/instance_test.cc` describe the behaviour that would be consistent
with `Instance::GetBoundingBox` and `Layout::Flatten`. The contradictions are
also noted in the code, beside the transform members in
`geometry/instance.h`.

**Deferred:** revisit this after the other low-hanging fruit in this plan are
done (phases 0 and 3-5). Resolving it means choosing one convention, checking
it against how VLSIR writes GDS rotations, and possibly changing generator
output, so it is a larger change than the other fixes here.

- [ ] `Instance::GeneratePorts` converts the rotation to clockwise before
      passing it to a counter-clockwise function, so ports on instances
      rotated 90 or 270 degrees turn the opposite way to the instance's
      shapes. `Layout::ToVLSIRLayout` also passes the ccw value as VLSIR's
      `rotation_clockwise_degrees` (see the FIXME there), so which one matches
      the GDS depends on how VLSIR interprets it. Only 180-degree rotations are
      used in generators today, where the two agree.
      `DISABLED_Ports_Rotated90MatchBoundingBox`
- [ ] `GeneratePorts`, `ApplyInstanceTransforms(ShapeCollection*)` (used by
      all `Copy*Shapes*`) and `ApplyInstanceTransforms(Point*)` (used by
      `GetPoint`) ignore `reflect_vertical_`, which `GetBoundingBox` and
      `Flatten` apply. VLSIR also says rotation applies after reflection,
      while BFG reflects after rotating.
      `DISABLED_Ports_ReflectedMatchBoundingBox`,
      `DISABLED_CopyAllShapes_ReflectedMatchesBoundingBox`,
      `DISABLED_GetPoint_Reflected`

## Observations (not yet confirmed as bugs)

- `Port::Compare` orders by `lower_left` only, so a `PortSet` silently keeps
  just one of several ports at the same position, even on different nets or
  layers.
- `Layout::CopyConnectableShapes` never stops at `max_depth` 0, unlike the
  other `Copy*` methods, so its depth is effectively off by one.
- `Layout::FlipHorizontal`/`FlipVertical` and the `ShapeCollection` versions
  only flip in place when the bounding box starts at the origin; `PolyLine`'s
  version flips in place anywhere.

## Later

- [ ] Resolve the instance rotation and reflection contradictions (see Open
      questions), then enable the 4 disabled tests in
      `geometry/instance_test.cc`.

## Out of scope

`main.cc`, `router_service_*`, `router_session.cc`: not linked into
`bfg_test`; better covered by integration tests.

## Log

- 2026-11-21: Measured baseline coverage. Wrote this plan.
- 2026-11-21: Phase 1 done: 60 new tests, 6 disabled tests marking bugs.
- 2026-11-21: Fixed the 6 bugs found in phase 1. 490 tests pass.
- 2026-11-21: Phase 2 done: 59 new tests; fixed 6 bugs; rotation/reflection
  questions left open. 549 tests pass.
- 2026-11-22: Noted rotation/reflection contradictions in the code; deferred
  fixing them until the other low-hanging fruit are done.
- 2026-11-22: Phase 0 done: `BFG_COVERAGE` option and `scripts/coverage.sh`.
