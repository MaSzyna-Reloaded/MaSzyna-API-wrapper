extends MaszynaGutTest

## The simulation has one clock, MaszynaRuntime's (Timer::UpdateTimers(), Timer.cpp:79-87): a frame
## advances it by its delta times the simulation speed, at most MAX_FRAME_TIME; the physics
## integrates exactly that, whole, in steps no longer than PHYSICS_STEP (drivermode.cpp:193-206).
## A machine that cannot keep up makes the simulation run slower than the wall clock; nothing is
## owed and nothing jumps.

## MaszynaRuntime::MAX_FRAME_TIME (Timer.cpp:84)
const MAX_FRAME_TIME:float = 1.0
const VELOCITY_MS:float = 10.0
const TRACK_LENGTH_M:float = 2000.0
const DOUBLE_SPEED:float = 2.0
const SHORT_FRAME:float = 0.1
## What the clock node adds between the calls of a test - a frame or two at most [s]
const FRAME_TOLERANCE:float = 0.05

var _track:RID = RID()
var _controller:VehicleController = null
var _vehicle:RID = RID()


func before_each() -> void:
    var curve:MaszynaTrackCurve = MaszynaTrackCurve.new()
    curve.p1 = Vector3.ZERO
    curve.p2 = Vector3(TRACK_LENGTH_M, 0.0, 0.0)
    _track = TrackManager.track_create()
    TrackManager.track_update_curves(_track, curve, null)
    TrackManager.track_update(_track, TrackManager.TRACK_NORMAL, "", 1.435)
    TrackManager.topology_rebuild()
    # a real vehicle, because the dynamics need a mass - an empty Mover integrates to NaN
    var model:VehicleModel = load("res://tests/fixtures/sm42_vehicle.tres") as VehicleModel
    _controller = build_vehicle("clock_test", model, VELOCITY_MS * 3.6)
    _controller.type_name = "test"
    # an unmanned vehicle is not simulated at all (Mover.cpp:4485) - see FINDINGS.md, 2026-09-23
    _controller.driver_type = VehicleController.DRIVER_HEAD
    _vehicle = _controller.get_rid()
    RailVehicleServer.vehicle_set_track(_vehicle, _track, 100.0, TrackManager.DIRECTION_NORMAL)
    await wait_idle_frames(1)


func after_each() -> void:
    MaszynaRuntime.simulation_speed = 1.0
    if TrackManager.track_exists(_track):
        TrackManager.track_free(_track)
    TrackManager.topology_rebuild()
    _controller = null


func _travelled() -> float:
    return RailVehicleServer.vehicle_get_transform(_vehicle).origin.x


## A stalled frame advances the clock by MAX_FRAME_TIME and no more - the rest is not owed - and
## the physics integrates all of that one second, not a budget of it.
func test_a_long_frame_is_capped_and_integrated_whole() -> void:
    var time_before:float = MaszynaRuntime.get_simulation_time()
    var before:float = _travelled()
    MaszynaRuntime.advance(5.0)
    assert_almost_eq(MaszynaRuntime.get_simulation_time() - time_before, MAX_FRAME_TIME, 0.0001,
            "the clock takes one second of the five")
    assert_almost_eq(_travelled() - before, MAX_FRAME_TIME * VELOCITY_MS, VELOCITY_MS * FRAME_TOLERANCE,
            "and the vehicle drove that whole second")


## The simulation speed scales the frame, for the clock and the vehicles alike.
func test_the_speed_scales_the_clock_and_the_physics() -> void:
    MaszynaRuntime.simulation_speed = DOUBLE_SPEED
    var time_before:float = MaszynaRuntime.get_simulation_time()
    var before:float = _travelled()
    MaszynaRuntime.advance(SHORT_FRAME)
    var seconds:float = SHORT_FRAME * DOUBLE_SPEED
    assert_almost_eq(MaszynaRuntime.get_simulation_time() - time_before, seconds, 0.0001)
    assert_almost_eq(_travelled() - before, seconds * VELOCITY_MS, VELOCITY_MS * FRAME_TOLERANCE,
            "the vehicle drove the simulated seconds, not the real ones")


## The time of day runs with the same seconds.
func test_the_time_of_day_runs_by_the_clock() -> void:
    var hours_before:float = MaszynaRuntime.time_of_day
    MaszynaRuntime.advance(MAX_FRAME_TIME)
    assert_almost_eq(fposmod(MaszynaRuntime.time_of_day - hours_before, 24.0), MAX_FRAME_TIME / 3600.0,
            FRAME_TOLERANCE / 3600.0)


## Paused, the clock stands, and so does everything that reads it.
func test_the_clock_stands_while_paused() -> void:
    MaszynaRuntime.pause()
    var time_before:float = MaszynaRuntime.get_simulation_time()
    var before:float = _travelled()
    await wait_idle_frames(2)
    assert_eq(MaszynaRuntime.get_simulation_time(), time_before)
    assert_eq(_travelled(), before)
    MaszynaRuntime.unpause()
