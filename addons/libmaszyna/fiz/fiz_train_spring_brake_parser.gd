@tool
extends RefCounted
class_name FizTrainSpringBrakeParser

## SpringBrake: section parser -> TrainSpringBrake. Registered directly in
## FizTrainControllerInstancer's section table. Key mapping confirmed directly against
## TrainSpringBrake::_do_update_internal_mover's own field-name comments (which mirror the
## original MaSzyna short key names almost 1:1, e.g. MaxSetPressure/ResetPressure/PressureOff).


func create_node() -> TrainSpringBrake:
    return TrainSpringBrake.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainSpringBrake", node)

    if kv.has("Volume"):
        node.set_spring_actuator_chamber_volume(FizLineUtil.get_float(kv, "Volume"))
    if kv.has("MBF"):
        node.set_pressure_force_coefficient(FizLineUtil.get_float(kv, "MBF"))
    if kv.has("MaxSP"):
        node.set_spring_actuator_preload_pressure(FizLineUtil.get_float(kv, "MaxSP"))
    if kv.has("ResetP"):
        node.set_max_spring_actuator_filling_force(FizLineUtil.get_float(kv, "ResetP"))
    if kv.has("MinFP"):
        node.set_spring_full_balance_pressure(FizLineUtil.get_float(kv, "MinFP"))
    if kv.has("PressOff"):
        node.set_brake_signal_released_state_pressure(FizLineUtil.get_float(kv, "PressOff"))
    if kv.has("PressOn"):
        node.set_brake_signal_braked_state_pressure(FizLineUtil.get_float(kv, "PressOn"))
    if kv.has("ValveOnArea"):
        node.set_actuator_discharge_valve_cross_section(FizLineUtil.get_float(kv, "ValveOnArea"))
    if kv.has("ValveOffArea"):
        node.set_actuator_charge_valve_cross_section(FizLineUtil.get_float(kv, "ValveOffArea"))
    if kv.has("ValvePNBArea"):
        node.set_pneumatic_brake_valve_cross_section(FizLineUtil.get_float(kv, "ValvePNBArea"))
    if kv.has("MTC"):
        node.set_required_coupler_connection_method(FizLineUtil.get_int(kv, "MTC"))
