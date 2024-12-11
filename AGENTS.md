# AGENTS

## Mover Integration Rules

- Do not patch `src/maszyna/McZapkie/Mover.cpp` by changing the physics or coupler-force formulas unless the same change is first justified against `main`.
- When behavior diverges, assume the bug is in the wrapper/integration layer first:
  `TrackManager`, `LegacyRailVehicle`, `TrainSet3D`, track offsets, neighbour mapping, or coupler state exported into the mover.
- If `Mover` must be touched, prefer refactoring or adapting data structures to match the original expectations, not changing the underlying equations.
- Before changing `Mover` semantics, compare the relevant fragment with `main` and treat `main` as the source of truth for calculations.

## Track / Offset Semantics

- Keep one consistent offset convention across the wrapper:
  higher `track_offset` is ahead, lower `track_offset` is behind.
- In `TrainSet3D`, the first vehicle child is the lead vehicle at `start_track_offset`; trailing vehicles must be placed by subtracting spacing.
- Editor preview and runtime placement must use the same offset sign and same authored-order semantics.

## Coupler / Neighbour Debugging

- Do not guess coupler-side bugs from motion alone. First inspect runtime state.
- Use exported debug state to compare:
  `track_offset`, `track_id`, `connected_number`, `front_neighbour_*`, and `rear_neighbour_*`.
- When debugging side interpretation, distinguish two layers:
  explicit mover coupling in `Couplers[*]` vs virtual same-track neighbours in `Neighbours[*]`.
- Add or extend debug dumps in wrapper state before changing side logic, so the issue can be verified from the existing debug panel.

## Safe Change Strategy

- Prefer small, isolated fixes in wrapper code over broad changes across placement, neighbour lookup, and mover internals at once.
- After each behavioral fix, rebuild and verify with the three-vehicle scenario:
  `SM42`, `Wagon1`, `Wagon2`, including init, forward motion, collision, and decouple.
