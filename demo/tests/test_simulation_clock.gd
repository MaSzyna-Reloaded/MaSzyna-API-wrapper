extends MaszynaGutTest

## The simulation has one clock, MaszynaRuntime's (Timer::UpdateTimers(), Timer.cpp:79-87): a frame
## advances it by its delta, at most MAX_FRAME_DELTA, times the simulation speed; the physics
## integrates exactly that, whole, in steps no longer than PHYSICS_STEP (drivermode.cpp:193-206),
## and the drivers and events get it in slices of at most MAX_SLICE_TIME. The simulation speed is
## honoured whatever the frames cost; a machine under 1 / MAX_FRAME_DELTA fps runs it slower,
## nothing is owed and nothing jumps.

## MaszynaRuntime::MAX_FRAME_DELTA
const MAX_FRAME_DELTA:float = 0.25
## MaszynaRuntime::MAX_SLICE_TIME
const MAX_SLICE_TIME:float = 0.1
## x4: one capped frame is a second of simulation
const SECOND_A_FRAME_SPEED:float = 4.0
## x100: the speed is honoured, not capped at the original's second a frame
const FAST_SPEED:float = 100.0
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


## A stalled frame counts MAX_FRAME_DELTA of real time and no more - the rest is not owed - times
## the speed, and the physics integrates all of it, not a budget of it.
func test_a_long_frame_is_capped_and_integrated_whole() -> void:
    MaszynaRuntime.simulation_speed = SECOND_A_FRAME_SPEED
    var time_before:float = MaszynaRuntime.get_simulation_time()
    var before:float = _travelled()
    MaszynaRuntime.advance(5.0)
    var seconds:float = MAX_FRAME_DELTA * SECOND_A_FRAME_SPEED
    assert_almost_eq(MaszynaRuntime.get_simulation_time() - time_before, seconds, 0.0001,
            "the clock takes a quarter of the five real seconds, sped up")
    assert_almost_eq(_travelled() - before, seconds * VELOCITY_MS, VELOCITY_MS * FRAME_TOLERANCE,
            "and the vehicle drove that whole time")


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


## A long frame is simulated in slices of at most MAX_SLICE_TIME, each announced: a driver or an
## event reacts in it as it would in a frame that short.
func test_a_long_frame_is_announced_in_short_slices() -> void:
    MaszynaRuntime.simulation_speed = SECOND_A_FRAME_SPEED
    var slices:Array[float] = []
    var record:Callable = func(seconds:float) -> void: slices.append(seconds)
    MaszynaRuntime.simulation_advanced.connect(record)
    MaszynaRuntime.advance(MAX_FRAME_DELTA)
    MaszynaRuntime.simulation_advanced.disconnect(record)

    assert_eq(slices.size(), roundi(MAX_FRAME_DELTA * SECOND_A_FRAME_SPEED / MAX_SLICE_TIME), "ten slices of a second")
    for seconds:float in slices:
        assert_almost_eq(seconds, MAX_SLICE_TIME, 0.0001)


## The time of day runs with the same seconds.
func test_the_time_of_day_runs_by_the_clock() -> void:
    var hours_before:float = MaszynaRuntime.time_of_day
    MaszynaRuntime.advance(MAX_FRAME_DELTA)
    assert_almost_eq(fposmod(MaszynaRuntime.time_of_day - hours_before, 24.0), MAX_FRAME_DELTA / 3600.0,
            FRAME_TOLERANCE / 3600.0)


## x100 is x100: a 60 fps frame is well past the original's second a frame.
func test_the_speed_is_honoured_past_a_second_a_frame() -> void:
    MaszynaRuntime.simulation_speed = FAST_SPEED
    var time_before:float = MaszynaRuntime.get_simulation_time()
    var frame:float = 1.0 / 60.0
    MaszynaRuntime.advance(frame)
    assert_almost_eq(MaszynaRuntime.get_simulation_time() - time_before, frame * FAST_SPEED, 0.0001)


## Paused, the clock stands, and so does everything that reads it.
func test_the_clock_stands_while_paused() -> void:
    MaszynaRuntime.pause()
    var time_before:float = MaszynaRuntime.get_simulation_time()
    var before:float = _travelled()
    await wait_idle_frames(2)
    assert_eq(MaszynaRuntime.get_simulation_time(), time_before)
    assert_eq(_travelled(), before)
    MaszynaRuntime.unpause()
