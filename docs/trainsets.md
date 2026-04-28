---
layout: page
title: "Trainset Integration with Mover"
---

This document describes the integration contract between the wrapper and the original MaSzyna Mover when a consist contains more than one vehicle.

The goal is to preserve the original `DynObj` semantics:

1. Neighbours are resolved from a stable snapshot of the consist.
2. Forces are computed for all vehicles before any vehicle publishes its new position.
3. Movement is published only after all vehicles finish `ComputeMovement()`.

## Authoritative tick order

Each frame, the wrapper must process all active `TrainController` instances in this order:

1. Update dirty mover configuration.
2. Resolve and publish the current track state for every vehicle.
3. Synchronize mover neighbours for every vehicle.
4. Run `ComputeTotalForce()` for every vehicle.
5. Run `ComputeMovement()` for every vehicle and store a pending movement result.
6. Publish the pending track position and mover location for every vehicle.

Do not collapse these phases into a per-vehicle `force -> move -> publish` loop. That reintroduces order-dependent consist behaviour.

## Position ownership

For trainsets, the wrapper owns the published on-track position.

- `track_rid`, `track_id`, and `track_offset` are the authoritative published location.
- `RailVehicle3D` reads the published track state and updates the scene transform from it.
- `Mover::Loc` is an integration detail mirrored from the published track state.

`RailVehicle3D` can also drive visual wheel and bogie animation from the published mover state:

- `front_rolling_wheel_paths`, `powered_wheel_paths`, and `rear_rolling_wheel_paths` are manual `NodePath` arrays for the three wheel groups used by the original simulator.
- `front_bogie_path` and `rear_bogie_path` are optional manual `NodePath` bindings for the two bogies.
- when both bogies are configured, the vehicle body position and yaw are derived from the line between the sampled bogie positions on the track.
- when no bogies are configured, the vehicle falls back to the default transform derived directly from `track_offset`.

In normal on-track runtime, scene code must not push a fresh position back into the mover each frame.

## Effective movement per frame

The published movement distance for a vehicle is:

`external_move_accumulator + ComputeMovement(...)`

`external_move_accumulator` exists for wrapper-driven displacement that must be visible to coupler logic in the current frame.

The accumulator must be:

- copied into `mover->dMoveLen` before `ComputeTotalForce()`,
- added to the value returned by `ComputeMovement()` when preparing the pending publish result,
- cleared only after the publish phase finishes.

## Neighbours and couplers

There are two neighbour sources and they must not be mixed conceptually:

- Physical coupler connection:
  `Couplers[*].Connected` is the authoritative link for already coupled vehicles.
- Track lookup:
  `Neighbours[*]` may point to the nearest potential collision source when there is no explicit coupler connection on that end.

When a coupler is connected, the wrapper must treat the connected mover as the neighbour source for that end and derive the distance from `TMoverParameters::CouplerDist(...)`.

## Track and path semantics

The wrapper uses one offset convention everywhere:

- higher `track_offset` is ahead,
- lower `track_offset` is behind.

Path transitions are resolved only in the publish phase. `ComputeMovement()` returns a distance along the current frame, but the wrapper is responsible for resolving that distance into:

- the destination segment,
- the local offset on that segment,
- the published world position.

This keeps the Mover math unchanged while allowing the wrapper to span chained path tracks.

## Rules for future changes

Do:

- compare trainset movement changes against `~/src/maszyna/DynObj.cpp`,
- keep force calculation and movement publication as separate global phases,
- debug consist issues using `track_offset`, `track_id`, coupler state, and neighbour state together.

Do not:

- reintroduce `movement_delta` as a public movement contract,
- publish new `track_offset` or `Mover::Loc` during the force phase,
- change Mover coupler or force formulas to compensate for wrapper ordering bugs,
- let `RailVehicle3D` or other scene nodes become the authoritative source of on-track mover position.
