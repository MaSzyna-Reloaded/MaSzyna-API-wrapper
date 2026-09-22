extends MaszynaGutTest
## GenericVehicleComponent is the gateway a modder writes a vehicle component in GDScript through.
## Nothing covered it before, so the whole contract is asserted here: the tick, both dumps, the
## way back to the vehicle, and command registration.

const ProbeComponent: GDScript = preload("fixtures/probe_vehicle_component.gd")

var _controller: VehicleController = null
var _probe: GenericVehicleComponent = null


func before_each() -> void:
    _controller = VehicleController.new()
    _controller.train_id = "GenericComponentTest"
    _probe = ProbeComponent.new()
    _probe.name = "ProbeComponent"
    _controller.add_child(_probe)
    add_child(_controller)
    await wait_idle_frames(2)


func after_each() -> void:
    remove_child(_controller)
    _controller.free()
    _controller = null
    _probe = null


func test_the_script_is_ticked_with_the_frame_delta() -> void:
    var before: int = _probe.process_calls
    await wait_idle_frames(2)
    assert_gt(_probe.process_calls, before, "_process_component runs while the component is enabled")
    assert_gt(_probe.last_delta, 0.0, "it is handed the frame delta")


func test_the_scripts_keys_reach_the_vehicle_state_dump() -> void:
    var state: Dictionary = _controller.get_state()
    assert_true(state.has("probe_process_calls"), "_get_component_state feeds the vehicle dump")
    assert_true(state.has("velocity"), "the vehicle's own keys are there too")


func test_the_scripts_keys_reach_the_vehicle_config_dump() -> void:
    _probe.apply_config()
    var config: Dictionary = _controller.get_config()
    assert_eq(config.get("probe_config_key"), "probe", "_get_component_config feeds the config dump")


func test_the_component_reaches_its_vehicle() -> void:
    assert_same(_probe.get_controller(), _controller, "get_controller returns the owning vehicle")
    assert_true(_probe.get_vehicle_state().has("velocity"), "get_vehicle_state is the whole vehicle's")


func test_a_command_registered_from_the_script_is_received() -> void:
    TrainSystem.send_command(_controller.train_id, "probe_command", null, null)
    assert_eq(_probe.commands_received, 1, "register_command wired the script's handler")


## A disabled component is left out of the tick and out of the dump, the way a native one is.
func test_a_disabled_component_neither_ticks_nor_publishes() -> void:
    _probe.enabled = false
    await wait_idle_frames(2)
    var before: int = _probe.process_calls
    await wait_idle_frames(2)
    assert_eq(_probe.process_calls, before, "a disabled component is not ticked")
    assert_false(_controller.get_state().has("probe_process_calls"), "nor does it publish state")
