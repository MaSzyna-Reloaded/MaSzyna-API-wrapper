extends MaszynaGutTest

## Simulation time is never dropped: the scenario's events and multiplayer are driven by it, so a
## simulation that ran slower than the clock would drift out of both. A frame that cannot be
## integrated now - the sub-step has to stay at or below PHYSICS_STEP, or stiff coupler springs
## kick - leaves the rest owed, and the frames that follow pay it off (see FINDINGS.md and
## RailVehicleServer::step_frame()).

## MAX_PHYSICS_ITERATIONS * PHYSICS_STEP: the most one frame can integrate without the sub-step
## growing past what the couplers were tuned for.
const FRAME_BUDGET:float = 0.2
const VELOCITY_MS:float = 10.0
const TRACK_LENGTH_M:float = 2000.0

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
    _controller = build_vehicle("catch_up_test", model, VELOCITY_MS * 3.6)
    _controller.type_name = "test"
    # an unmanned vehicle is not simulated at all (Mover.cpp:4485) - see FINDINGS.md, 2026-09-23
    _controller.driver_type = VehicleController.DRIVER_HEAD
    _vehicle = _controller.get_rid()
    RailVehicleServer.vehicle_set_track(_vehicle, _track, 100.0, TrackManager.DIRECTION_NORMAL)
    await wait_idle_frames(1)
    # the debt belongs to the server, so it outlives a test - restart the stepping to clear it
    RailVehicleServer.set_stepping_enabled(false)
    RailVehicleServer.set_stepping_enabled(true)


func after_each() -> void:
    if TrackManager.track_exists(_track):
        TrackManager.track_free(_track)
    TrackManager.topology_rebuild()
    _controller = null


func _travelled() -> float:
    return RailVehicleServer.vehicle_get_transform(_vehicle).origin.x


## A stalled frame is not integrated whole: what is over the budget waits rather than being taken
## with a sub-step several times larger than the couplers can stand.
func test_a_long_frame_integrates_only_what_it_can() -> void:
    var before:float = _travelled()
    RailVehicleServer.step_frame(1.0)
    var moved:float = _travelled() - before
    assert_almost_eq(moved, FRAME_BUDGET * VELOCITY_MS, 0.2, "one frame takes at most its budget")


## ...and the rest is not lost: the frames after it pay the debt off, so the distance travelled
## matches the time that passed on the clock.
func test_the_debt_is_paid_off_and_no_time_is_lost() -> void:
    var before:float = _travelled()
    RailVehicleServer.step_frame(1.0)
    for i in range(20):
        RailVehicleServer.step_frame(0.0166)
    var elapsed:float = 1.0 + 20 * 0.0166
    assert_almost_eq(
        _travelled() - before, elapsed * VELOCITY_MS, 0.5,
        "every second the clock counted is a second the vehicle drove"
    )
