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

## Phase 3: routing (done 2026-11-22)

- [x] New `routing/routing_grid_test.cc` with a shared met1/met2 Sky130 grid
      fixture (`routing/routing_grid_test_fixture.h`): `ConnectLayers`,
      `FindViaStack(Cost)`, `LayersReachableByVia`,
      `VerticesAreTooCloseForVias`, `ShortestPath`, `AddRouteBetween`
      (crossing nets, walled-in failure), `AddRouteToNet`, `AddBlockage`,
      `GenerateLayout`. `RoutingGridTest` is a friend of `RoutingGrid`.
- [x] New `routing/routing_path_test.cc`: construction, `Cost`,
      `SpannedVertices*`, `AddPortMidway`, legalisation and edge merging,
      `ToPointsAndLayers`, `CheckStillAvailable` (other-layer crossing is fine;
      same-layer use by another net fails; same net is fine), `Describe`
- [x] `route_manager_test.cc`: `Connect`, `ConnectMultiplePorts` (ports and
      sets), `ConnectToNet`, `Solve` (serial and parallel),
      `GetOrderAndResult`, `SummariseStatuses`, descriptions
- [x] `routing_track_test.cc`: vertex lookup, edges, `VerticesInSpan`,
      `RemoveEdge`, `RemoveVertex`, temporary blockages, `Describe`
- [x] `routing_blockage_cache_test.cc`: net exceptions, `IsEdgeBlocked`,
      `CancelBlockages` by net and layer, child caches,
      `ValidAgainstKnownBlockages`, `Summary`
- [x] New `physical_properties_database_test.cc`: `FindViaStackImpl` on
      hand-built graphs, Sky130 via stacks, reachability, encaps, routing
      layer/via info, pin layers
- [x] New `routing/routing_track_direction_test.cc`
- [ ] Still untested directly: `RoutingPath` notch checks
      (`CheckForNotchesToPerpendicularEdges`) and abbreviation
      (`AbbreviateOnce`, `InstallAbbreviatingJog`), which need hand-built
      layouts that provoke them; `RoutingTrack::HealEdges` (never called)

| File | Lines before | Lines after |
|---|---|---|
| `routing/routing_grid.cc` | 18% | 42% |
| `routing/routing_path.cc` | 0% | 46% |
| `routing/route_manager.cc` | 26% | 80% |
| `routing/routing_track.cc` | 47% | 66% |
| `routing/routing_blockage_cache.cc` | 54% | 84% |
| `physical_properties_database.cc` | 42% | 83% |

Overall: 24.8% -> 32.6% of lines, 44.7% -> 56.9% of functions. 622 tests pass.

## Phase 4: generator smoke tests (done 2026-11-22; to be revisited)

**Revisit:** generator tests (atoms and tiles) need to be higher level than
unit tests, running external tools: DRC, LVS and simulation. The smoke tests
below only check that generation completes and produces something
plausible; it is not yet clear how useful they are as unit tests. Decide
what to keep once the higher-level flow exists.

`atoms/generator_test_fixture.h` sets up a `DesignDatabase` the way `main()`
does (Sky130 technology, primitives and standard cells; CMake copies the
packages into the test data directory) and checks that a generated cell has a
non-empty layout and, where circuits are implemented, ports, with pins for
named ports.

- [x] Atoms (`atoms/generators_test.cc`, in `bfg_test`): `Sky130Buf`,
      `Sky130Carry1`, `Sky130Decap`, `Sky130Dfxtp`, `Sky130HdMux21`,
      `Sky130InterconnectMux1`, `Sky130InterconnectMux2` (the two
      configurations `ReducedTile` uses), `Sky130Mux`, `Sky130PassiveMux3`
      and `Sky130SwitchComplex` (layout only; their circuits are
      unimplemented), `Sky130SplitBuffer`, `Sky130Tap`,
      `Sky130TransmissionGate`, `Sky130TransmissionGateStack`, `Sky130Xor2`,
      `LayoutDemo`. Together under 2 s.
- [x] `InterconnectWireBlock` (`tiles/interconnect_wire_block_test.cc`, in
      `bfg_test`): the default has no wires, so a one-bundle channel is used.
- [x] Other tiles (`tiles/slow_generators_test.cc`, in the new
      `bfg_slow_test`, which ctest does not run): each takes minutes even at
      `-O2`, so they are run by hand. At `-O2`: `LutB` passes (~80 s); `S44`
      generates (~200 s; layout only, its circuit has no ports);
      `Interconnect` takes over 4 minutes; `Lut` aborts in routing
      (disabled); `Slice` hit a naming bug (see below); `ReducedTile` and
      `Fabric` need an edge list CSV that is not in the repository
      (disabled).
- [ ] `Gf180McuMux` (needs a GF180 fixture)
- [ ] Device-count checks against parameters
- [x] `Sky130TransmissionGateTest.ViaLocations` (empty) and the 3 disabled
      tests from before this work: left as they were

## Phase 5: placement helpers and databases (done 2026-11-22)

- [x] `row_guide.cc` (new `row_guide_test.cc`): placement front and back,
      blank space, vertical centring, rotation, moves, taps, span, circuit
      instances
- [x] `memory_bank.cc` (new `memory_bank_test.cc`): row creation and rotation,
      vertical abutment, left/right alignment, moves, `InsertRowZero`, taps
- [x] `checkerboard_guide.cc` (new `checkerboard_guide_test.cc`)
- [x] `circuit.cc` (new `circuit_test.cc`), `design_database.cc` (new
      `design_database_test.cc`)

| File | Lines before | Lines after |
|---|---|---|
| `row_guide.cc` | 0% | 93% |
| `memory_bank.cc` | 0% | 92% |
| `checkerboard_guide.cc` | 0% | 99% |
| `circuit.cc` | 0% | 79% |
| `design_database.cc` | 0% | 78% |
| `atoms/sky130_mux.cc` | 0% | 92% |
| `atoms/sky130_interconnect_mux1.cc` | 0% | 92% |
| `atoms/sky130_split_buffer.cc` | 0% | 94% |

Overall (`bfg_test` only): 32.6% -> 67.5% of lines, 56.9% -> 78.7% of
functions. 693 tests pass.

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

## Bugs found in phase 3

Fixed on branch `test-coverage`, with their tests enabled:

- [x] `RoutingPath::SpannedVerticesWithoutVias` passed unsorted input to
      `std::set_difference`. Feeds layout-generation patching, so output may
      change. (1ed065c)
- [x] `RouteManager::SummariseStatuses` always returned an error. (febe11f)
- [x] `RouteManager::Solve` returned an empty list. (0b4f019)
- [x] `FindViaStackImpl` read a dangling reference after `queue.pop()`, so it
      expanded the wrong layer and could return bad stacks or crash. Used by
      both `PhysicalPropertiesDatabase` and `RoutingGrid`. (675dc81)
- [x] `PhysicalPropertiesDatabase::GetRoutingViaInfo` crashed instead of
      returning nullopt for unconnected layers (reachable from
      `RouterSession`). (d125de7)
- [x] `GetRoutingLayerInfoOrDie` dereferenced an empty optional. (cfc5377)
- [x] `RoutingTrack::ReportAvailableEdges` wrote past the end of its output
      vector. No callers yet. (2694d19)
- [x] `RoutingTrack::RemoveTemporaryBlockage` returned true when nothing was
      removed. (8865641)

## Bugs found in phases 4 and 5

Fixed on branch `test-coverage`, with their tests enabled:

- [x] `RowGuide::GetBoundingBox` ignored all but the first and last instance.
      (2884c3f)
- [x] `RowGuide::AvailableRightSpanUpTo` dereferenced an unset tap cell.
      (a75f6d0)
- [x] `MemoryBank::AlignLeft`/`AlignRight` started from x = 0, so moved banks
      snapped back. (ff6f870)
- [x] `CheckerboardGuide` made stripes, not a checkerboard, for odd column
      counts. (8122ca7)
- [x] `Circuit` left `parent_cell_`, `type_` and `unnamed_net_count_`
      uninitialised; unnamed circuits dereferenced garbage. (cc98f67)
- [x] `Layout` left `parent_cell_` uninitialised. (abe2ec7)
- [x] `Circuit::AddPort` warned about width mismatches only when widths
      matched. (950cd2e)
- [ ] `Slice` names every LUT instance `lut_i`, a fatal duplicate. Fix
      (`"_", i`) being verified in `bfg_slow_test`.

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
- `RouteManager::Solve` ignores the statuses from running the orders and
  always returns OK, though `LutB` checks it to collect routing errors.
  Propagating them would change generator behaviour.
- `GetRoutingViaInfoOrDie` sets the via height from `via_width`. Sky130 vias
  are square; the GF180 setup only sets `via_width` (`via_height` is 0), so
  switching to `via_height` would break GF180.
- `FindCostedLayersReachableThroughOneVia` takes `via_cost` from the rules of
  the destination metal layer rather than the via layer between them. No
  layer sets `via_cost` today, so all costs are 0 either way.
- `FindViaStackImpl` uses a `std::priority_queue` whose comparator reads costs
  that change while elements are queued, and never re-queues a layer whose
  cost improves. Results are right on every graph tested.
- `RoutingBlockageCache::IsVertexBlocked` exempts `for_nets` from blockage
  users but not from direction inhibition, so a same-net rail still blocks a
  vertex on it.
- `RoutingTrack::MergeNewBlockage` erases merged blockages from its container
  without deleting them.
- `Sky130InterconnectMux2` aborts with the default `num_inputs` (6): a gate
  stack select port is connected twice. The configurations in use (7, and 5
  inside out) work; 3 hits an explicit check.
- `CheckerboardGuide` computes column widths assuming a template for every
  grid cell, but assigns templates only to occupied cells, so with several
  templates of different widths the columns can be sized from the wrong
  cells. It happens to be right for `LutB`'s 2 x 2 grid.
- `MemoryBank` creates new rows at x = 0 regardless of where the bank was
  moved (documented in the header as a TODO).
- `DesignDatabase::OrderCells` (private, unused) sorts with a comparator that
  is not a strict weak ordering. `LoadModule` builds and leaks a second
  `Circuit`. `Tile::GenerateIntoDatabase` ignores `ConsumeCell` failing on a
  duplicate name, leaking the new cell.
- `Lut` (the older LUT design) aborts in routing with default parameters.


## Later

- [ ] Replace or supplement the generator smoke tests with DRC, LVS and
      simulation checks (see Phase 4).
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
- 2026-11-22: Phase 3 done: 73 new tests; fixed 8 bugs. 622 tests pass.
- 2026-11-22: Phases 4 and 5 done; fixed 7 more bugs.
