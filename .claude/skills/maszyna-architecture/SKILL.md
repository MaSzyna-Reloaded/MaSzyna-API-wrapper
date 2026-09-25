---
name: maszyna-architecture
description: Decide where a piece of state, behaviour or API belongs in this wrapper's layers, and check that a change does not breach a boundary. Use before adding a class, a field, a signal, a state key or a server method; before moving logic between C++ and GDScript; when something needs a value that lives in another layer; when a node, a system and a server all look like plausible homes for the same thing; and when reviewing a diff for separation of concerns. Answers "which layer owns this", "may this layer know about that one", and "is this the same mistake we already made".
---

# MaSzyna Reloaded - layers, boundaries and how they are enforced

This wrapper is a stack of layers, and the whole architecture rework (#184) exists to make each
one replaceable on its own. Most defects recorded in `FINDINGS.md` are a layer that reached into
another: a feature parked on a node vanished when a second instancer appeared without nodes; a
sound trigger's counters lived on the vehicle; a getter advanced a filter, so the value depended
on how often it was read.

**The question that settles every argument about placement:** *if this layer were replaced
wholesale, would this field go with it?* If yes, it belongs there. If it would have to be kept
for somebody else, it belongs to that somebody.

## The map

Read top to bottom: each layer may use the ones below it and must know nothing of the ones above.

| Layer | Who | Owns | Must never |
|---|---|---|---|
| Backend | `src/maszyna/` (vendored `TMoverParameters`) | physical quantities, the original's own model | be edited. It is vendored; a divergence is ported around it, never into it |
| Backend adapters | `Mover*` classes (`MoverVehicleController`, `MoverVehicle<Domain>`, `MoverComponent`) | the only `TMoverParameters *` in the process | appear in any name, parameter or return type above this row |
| Vehicle model | `VehicleController` (an `Object`), `VehicleComponent` + `Vehicle<Domain>` interfaces | a vehicle's state, configuration and the operations that change them | name the backend; be a `Node`; hold anything only a drawing, sound or UI layer needs |
| Servers | `RailVehicleServer`, `TrackManager`, `TractionPowerServer`, `E3DRenderingServer`, `SceneryStreamingServer`, `PythonScreenServer`, and the GDScript rendering servers (`TrackRenderingServer`, `TractionRenderingServer`, `SceneryChunkRenderingServer`) | handles (`RID`), the placement and the composition of what they own, their own worker threads | hand out raw pointers, or hold state a single node could own |
| Systems | `TrainSystem`, `CabinSystem`, `TrainSoundSystem`, `MaterialManager`, `SceneryInstancer` | cross-cutting bookkeeping keyed by handle, and the vocabulary the data uses | duplicate state the model already has; drive per-frame work that a server could do natively |
| Nodes | `RailVehicle3D`, `VehiclePhysicsNode`, `Cabin3D`, `RailVehicleStepper`, cab widgets | what is drawn and what is in the scene tree | contain simulation; own a handle the server already owns; reach a vehicle by walking the tree |
| Importers | `src/parsers/`, `addons/libmaszyna/fiz/`, `.../mmd/`, `.../importer/` | turning FIZ, MMD, E3D and `.scn` into model objects | build scene trees as their output, or keep parse results in nodes |

Autoloads are listed in `demo/project.godot`; C++ singletons are registered in
`src/register_types.cpp`. A singleton is reached by its typed `get_instance()` and the result is
null-checked - never by node path, never by `call("name")`.

## The four boundaries that carry the design

**1. The backend never appears above its adapter.** No `Vehicle*.hpp` interface mentions
`TMoverParameters`, and only `Mover*` files hold one. Every one of the 21 `MoverVehicle<Domain>`
implementations has a `Vehicle<Domain>` interface of the same name, and none of those interfaces
names the backend. A component reaches the backend through `MoverComponent` - which is not
an `Object`, so it needs `dynamic_cast`, not `Object::cast_to<>`, and the two base pointers of one
object differ in address.

    grep -l TMoverParameters $(find src -name 'Vehicle*.hpp')   # must print nothing

**2. A public API takes handles, not pointers.** `RailVehicleServer` exposes `vehicle_create`,
`vehicle_free`, `vehicle_set_track`, `vehicle_get_transform`, `vehicle_dump_state` - `RID` in,
`Variant` out. Its two `VehicleController *` are private helpers (`RailVehicleServer.hpp:130,141`),
which is the rule working: pointers stay inside one class.

**3. State has one owner and one writer, and a getter only reads.** Values reach other layers
through `_fill_state_dictionary` / `_fill_config_dictionary` on the component that owns them
(`VehicleComponent.hpp:92,106`), composed by the server into one cached dump. Change detection,
filters and flag consumption belong in the tick, never in a fetch or a getter - the four
violations of this cost a day each (`FINDINGS.md`, 2026-09-22).

**4. Ordering is an event, never a retry.** The simulation steps before anything reads it
(`RailVehicleStepper`, `process_priority` = -100), and a value that is not there yet is an
ordering defect: react to `mover_config_changed`, do not raise a "try again next frame" flag.

## Where a new thing goes

* **A value the simulation produces** - a component's own field, published through its
  `_fill_state_dictionary`. Before reusing a config getter for it, grep the backend for an
  assignment to that field: a value the simulation writes is state, a value only the wrapper
  writes is configuration, and confusing the two is how a battery reported a voltage that never
  moved.
* **A value only one consumer needs** - that consumer's layer. Sound bookkeeping lives in the
  sound system; what a gauge shows lives in the cab; a counter a trigger compares against is not
  the vehicle's business.
* **A handle, a placement, a composition** - the server that owns the thing. The body transform is
  composed by `RailVehicleServer` from the two bogie pivots, not by the node that draws it; two
  writers agreed on every straight track in the game and disagreed on curves.
* **Something an instancer must honour** - the server, not the node that happens to create the
  instance. The moment a second instancer appeared without nodes, every feature parked on a node
  silently stopped existing.
* **A new component** - a `Vehicle<Domain>` interface plus a `Mover<Domain>` implementation, added
  with `add_component()` and reached with `get_component(VehicleComponentType::TYPE)`.
* **Recurring work** - C++ on `process_frame`, then a `Timer`, then `_process` with an
  accumulator. Never a bare per-frame `_process`, never a loop in one, and never wiring
  (`connect`, `get_node`, a path) inside one.

## Breaches that exist today

These are known and being removed. **They are not precedent** - do not copy their shape, and do
not cite them as "how this codebase does it". Counts measured 2026-09-25; re-measure before
relying on one.

* `TrainSystem.hpp:19,30` keeps `std::map<String, VehicleController *>` and hands the pointer out.
  The name-to-`RID` registry it should use lives on `RailVehicleServer`.
* `VehiclePhysicsNode::_build()` creates, owns and frees the controller; `vehicle_create()` inserts
  an empty placement. The server should allocate and own it. `RailVehicle3D.cpp:449` creates a
  second handle on top of that - a node that draws a vehicle should own no handle.
* `RailVehicle3D` still reads the whole dump four times (`:541`, `:553`, `:712`, `:1108` - the
  pantograph helpers, the roof light and the wiper positions), three of them per frame, instead of
  reading its components.
* `CabinSystem`'s whole vehicle-facing surface is keyed on `train_id:String`, not on the handle
  (`cabin_system.gd:97,118,122`).
* There are no update phases: the step order is a hand-written sequence in
  `RailVehicleServer::step_frame()` (`:803`), and `UpdatePhase` exists nowhere.
* The dump carries two key conventions - nine `prefix/key` namespaces against a majority of flat
  `component_key` names. 48 `state_property` values in the MMD catalog contain `/`, so this is a
  data contract, not a rename.

## Before committing a change to any of this

1. `grep -l TMoverParameters $(find src -name 'Vehicle*.hpp')` - the backend has not leaked.
2. `grep -n '\*' <server>.hpp` - no pointer crossed a public boundary.
3. Read every `get_*` the diff adds: it returns and does nothing else.
4. Read every `_process` the diff touches: no loop, no lookup, no wiring, nothing while idle.
5. Ask the placement question of each new field, out loud, and write the answer in the commit.

**And verify against the code, never against a document.** `TODO.md`'s stage list was written when
the first part of each stage landed and went stale; this file will too. Every count and line
number here was measured, and the next reader measures again - a claim about this codebase that
nobody re-ran is a guess with a citation.
