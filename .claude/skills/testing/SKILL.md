---
name: testing
description: Write, change, run and judge tests in this wrapper (GUT, demo/tests) and headless probes. Use before adding or editing a test, before "fixing" a test that went red, before running tests ahead of a commit, when a change touches physics (Mover wrapper, vehicle components, FIZ factory, cab logic), and when a headless probe has to prove a hypothesis. Answers "what may a test assert", "is this red test a regression", "how do I run just my test" and "why does my probe read frozen values".
---

# Testing

`AGENTS.md` ("Checks", "Before every commit") and `CODE_STYLE.md` ("Tests") bind as written; this
skill gathers them with what they cost us when they were broken.

## A red test is a suspected regression - ABSOLUTE

A test that turns red after a change is **distrusted, not fixed**. Before touching it:

1. Read what it guarded - its assertions and its setup - and say why that behaviour changed on
   purpose. If you cannot, the change is wrong, not the test.
2. Never make it pass by removing its setup or its assertion, by loosening a tolerance, or by
   giving it a value the code now happens to produce.
3. When the change moved a responsibility (a field to another component, a command to another
   owner), the test follows it to the new owner with the **same** assertion.

2026-10-04, b5e744f1: the battery moved from the controller to `RailVehiclePowerSupply`; five tests
lost `battery_voltage = 110` with nothing in its place and stayed green, because none of them
asserted the low voltage. The tester could not start an EU07, ED78 or 36WE. Breaking this rule
breaks the project's rules.

## Physics is tested end to end and blackbox - ABSOLUTE

- A change to a physics component - the Mover wrapper (`src/legacy/vehicles/`), a vehicle
  component (`src/vehicles/`), the FIZ factory (`addons/libmaszyna/legacy/fiz/`), the cab logic
  that commands them (`addons/libmaszyna/legacy/cabin/`) - is tested through the whole sequence a
  player goes through, not only the field that moved: battery -> low voltage
  (`power24_available`) -> pantographs -> relay reset -> main switch -> converter
  (`power110_available`) -> controller -> speed. A flag that is set (`battery_enabled`) proves
  nothing about what depends on it.
- Physics tests are **blackbox**: they assert on what components return (typed getters -
  `RailVehiclePowerSupply.get_power24_available()`, `RailVehicleEngine.get_main_switch_enabled()`)
  and/or on the state built into the dump (`get_state()`, `VehicleServer.vehicle_dump_state()`).
  Never on Mover internals, private members (`_name`) or the way a component got its value.
- Test the real entry point: commands through `vehicle_send_command`, cab controls through
  `CabinSystem.act` on a `LegacyCabinLogic` - a hand-built struct can pass while the real path is
  broken.

## What a test may use

- Only fixtures: `demo/tests/fixtures/`, `demo/tests/materials/`. Never the game directory
  (`scenery/`, `dynamic/`, `textures/`) - CI has none. A real vehicle is cut into a fixture (FIZ,
  minimal MMD, the scenery lines it needs).
- Only the public interface of the tested class (`CODE_STYLE.md`, "Tests"). Needing private access
  means the API is missing something.
- Never a bare `[]`/`{}` to a parameter typed `Array[T]`/`Dictionary[K, V]` - the call is refused
  silently and the test checks nothing.
- A regression test is proven both ways: red on the code before the fix, green after.
- No throwaway diagnostic script in `demo/tests/` - probes live in the scratchpad.

## Running

- Parse-check first: `godot-double --headless --path demo --check-only -s res://tests/<file>.gd`
  (5 s). A script that does not parse is skipped by GUT, `-gselect` then matches nothing and the
  whole directory runs until the timeout - it looks like a hang.
- One script at a time, only the ones you wrote or changed:
  `godot-double --headless --path demo -s addons/gut/gut_cmdln.gd -gdir=res://tests/ -gselect=<script> -gexit`
  (60 s). `-gtest=` does not filter. Never the whole suite.
- Redirect to a file and read the file - `| grep | head` kills the run with SIGPIPE.
- `godot-double`, never `godot` (double-precision extension).
- After every headless run `git status`: Godot rewrites `.tres`/`.tscn`; restore what you did not
  edit.

## Headless probe of a vehicle (scratchpad)

A probe reads frozen values - and "proves" anything - unless the vehicle is really simulated:

- it stands on a track: `RailVehiclePhysicsNode` plus a `RailVehicle3D` whose `controller_path`
  is set **before** it enters the tree (only `RailVehicleServer.vehicle_is_attached()` vehicles are
  stepped);
- the clock runs: a `SimulationRuntime` node is in the tree and the simulation is unpaused; check
  that `SimulationServer.simulation_get_time()` grows;
- it is driven (`PlayerServer.player_take_over_vehicle()` or
  `DriverSystem.vehicle_set_control_active()`), or a standing vehicle switches its physics off;
- waits are in time (`create_timer`), not in frames - a headless frame is microseconds.

A probe using autoloads (`CabinSystem`, `UserSettings`) runs as a scene
(`godot-double --headless --audio-driver Dummy --path demo <scratch>/probe.tscn`); a `-s` script
does not compile against them. Without a wire the step feeds 0 V to the pantographs at its end - a
voltage fed by hand does not reach the cab's own tick. For a regression with no obvious cause,
build the commit before it in a copy in the scratchpad (`git archive <rev>^ | tar -x`, symlink
`godot-cpp` and `vendor/*`, its own `build-debug`) and run the same probe on both: the first step
that differs is the regression.
