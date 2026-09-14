---
name: mover-parity-check
description: Diagnose Godot-side train behavior bugs (electrical relays, engine startup, main switch/wyłącznik szybki, pantograph, brakes, etc.) by tracing the exact logic in the vendored original MaSzyna C++ simulator (~/src/maszyna) and comparing it against this wrapper's port. Use whenever an in-game behavior looks wrong/inconsistent and the question is "does our port match the original simulator's physics/electrical model, and where does it diverge?"
---

# Mover parity check

This repo (`MaSzyna-API-wrapper`) ports the original MaSzyna train simulator's
C++ physics/electrical core (`TMoverParameters`, vendored untouched under
`src/maszyna/`, mirroring `~/src/maszyna`) into a Godot GDExtension, plus a
GDScript layer (FIZ import, MMD cabin import, engine/controller nodes,
servers) that drives it. Bugs reported as in-game behavior ("X doesn't turn
on", "Y resets unexpectedly", "indicator Z never lights") are almost always
one of:

1. A field/flag in the vendored `Mover.cpp`/`MOVER.h` that the wrapper reads
   or writes incorrectly, at the wrong time, or not at all.
2. A command/UI path (cab button, keybind, MMD label) that the original
   engine's `Train.cpp` wires up but this wrapper never implemented.
3. A FIZ/MMD data file whose values or defaults aren't parsed the way the
   original `LoadFIZ_*` functions parse them.

This skill is the repeatable workflow for pinning down which of these it is,
using the real vendored source as ground truth instead of guessing from the
wrapper's code alone.

## Step 1 — get the real symptom, not a paraphrase

Ask (or read from the operator's report) for the *exact* in-game
observation: which indicator lights, which doesn't, in what order, under
what action. If a live session is available, prefer a runtime state dump
(e.g. this project's console `get <vehicle-name>` command) over inference -
it gives exact boolean/numeric field values (`relay_ground`, `main_switch_enabled`,
`current_collector/pantograph_first_voltage`, etc.) matching this wrapper's
`_do_fetch_state_from_mover()`-exposed dictionary keys.

## Step 2 — find the flag in the vendored engine

Locate the responsible field/flag in `~/src/maszyna` (or this repo's
`src/maszyna/`, which must stay byte-identical to it - see the "never touch
vendored Mover" rule below):

```
grep -n "<FlagName>" ~/src/maszyna/McZapkie/MOVER.h
grep -n "<FlagName>\s*=" ~/src/maszyna/McZapkie/Mover.cpp
```

Read every assignment site, not just the first. Distinguish:
- **Per-tick recomputed** state (reassigned unconditionally every `Update()`)
  vs. **latched/manual-reset** state (only cleared by a specific
  `RelayReset()`/button call, e.g. `GroundRelay`, `FuseFlag`,
  `ConvOvldFlag`).
- What **gates** it (`MainSwitchCheck()`-style AND chains are common - every
  clause is a separate failure mode).
- What it **gates in turn** (e.g. `GroundRelay` blocks `MainSwitchCheck()`
  from ever returning true).

## Step 3 — find the real UI/command path in the original engine

Grep `~/src/maszyna/Train.cpp` for the `OnCommand_*` handler and the
`user_command::` table entry, then grep the vehicle's real `.mmd` file
(under `~/Games/Maszyna/dynamic/...`) for the cab control label that fires
it (`_bt:`, `_sw:`, etc.) - labels follow the `ggXxxButton`/`ggXxxButton`
naming visible in `Train.cpp`'s `Load()`. Some commands are keybind-only with
no cab mesh (e.g. `universalrelayreset1/2/3`) - check
`user_command::` bindings, not just `.mmd` labels, before concluding a
control doesn't exist in the original either.

Also check the vehicle's real `.fiz` (same directory) for whether the
relevant config key is actually present for *this* vehicle - many FIZ files
only `include` a shared base file and override a handful of keys; a feature
gated behind a FIZ key (e.g. `RelayResetButton1=`) that the specific vehicle
never sets may be intentionally absent for that vehicle, not a parsing bug.

## Step 4 — compare against this wrapper's port

- FIZ parsing: `addons/libmaszyna/fiz/fiz_train_*_parser.gd` - check the key
  is read, and that the **default when the key is absent** matches the
  original's `LoadFIZ_*` fallback (grep `extract_value(...,line)` /
  `starts.find(...)` in `Mover.cpp` for the exact default, e.g. `GroundRelayStart`
  defaults to `automatic` only for `dt_EZT`, `manual` otherwise -
  `Mover.cpp` ~line 10703).
- Command wiring: `src/engines/*.cpp`, `src/controllers/*.cpp` -
  `register_command(...)` calls and their handler bodies. Check the handler
  calls the *right* vendored method (e.g. `fuse_reset()` calls `FuseOn()`,
  the traction-motor-overload reset - it is **not** a ground-relay reset,
  despite both being "fuse-style" buttons).
- MMD → command mapping: `addons/libmaszyna/mmd/mmd_semantic_catalog.gd` -
  grep the MMD label name to confirm a real cab click reaches the command at
  all. A command that exists in C++ but has no catalog entry is unreachable
  from the cab (only from the console/tests).
- Per-frame vs. dirty-flag config: this codebase's `TrainController`/engine
  nodes distinguish `state` (recomputed every tick) from `config`
  (change-notified, applied only when dirty). A setter that only takes
  effect through the dirty-flag config path has **zero effect** if called
  every frame expecting continuous effect (a real bug pattern hit before in
  the pantograph-voltage work) - check whether the wrapper writes straight
  to the mover (like `pantograph()`, `set_pantograph_wire_voltage()`) or
  goes through `_do_update_internal_mover()` (dirty-gated).

## Step 5 — state the root cause against the original as ground truth

Frame the finding as: "the original engine does/doesn't do X here (cite the
exact `Mover.cpp`/`Train.cpp` line and condition); this wrapper's port
does/doesn't match that (cite the wrapper file/line); the divergence is
<parsing default | missing command | missing MMD mapping | timing/dirty-flag
bug>." Don't stop at "the wrapper's code looks wrong" without checking what
the original actually does in the same situation - some surprising-looking
wrapper behavior (e.g. a relay staying latched off until a specific button
press) is faithful reproduction of real prototype electrical design, not a
bug.

## Hard constraints

- **Never edit `src/maszyna/` (vendored Mover).** All integration lives in
  the wrapper's own engine/controller classes, writing to the mover's public
  fields from outside - exactly like the original's own `DynObj.cpp`/
  `Train.cpp` split from `Mover.cpp`. If a genuinely vendored file must
  change, stop and ask first.
- Don't propose a fix that expands scope (new config fields, new state) until
  the actual root cause is confirmed against the vendored source - a missing
  command/mapping is a small, targeted addition, not a redesign.
