extends RefCounted
class_name LegacyCabinJointController

## Joint master controller (MMD "jointctrl:", e.g. SM42's nastawnik): moving it forward adjusts
## power, backward past the no-power position applies the local brake. Ported from the cab layer:
## TTrain::OnCommand_mastercontrollerincrease/decrease (Train.cpp:1094-1146) with the joint
## controller branches of OnCommand_independentbrakeincrease/decrease (Train.cpp:1449-1510).
## The handle shows controller_joint_position (Train.cpp:7699-7714).

const CONTROL:StringName = &"jointctrl"

var _train_id:String
var _cab:int


func control_ids() -> Array[StringName]:
    return [CONTROL]


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, CONTROL, _joint_controller)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, CONTROL, _joint_controller)


func _joint_controller(state:CabinState, action:StringName, _value:Variant) -> Variant:
    var vehicle:Dictionary = state.vehicle_state()
    if action == &"increase":
        # Train.cpp:1098 - an applied local brake is released first
        if float(vehicle.get("brake_local_position_normalized", 0.0)) > 0.0:
            return state.send_vehicle_command("local_brake_decrease")
        return state.send_vehicle_command("main_controller_increase")
    if action == &"decrease":
        # Train.cpp:1138 - below the no-power position the handle applies the local brake
        if vehicle.get("main_no_power_pos", false):
            return state.send_vehicle_command("local_brake_increase")
        return state.send_vehicle_command("main_controller_decrease")
    return null
