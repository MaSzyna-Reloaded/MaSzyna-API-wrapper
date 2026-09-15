@tool
extends RefCounted
class_name FizTrainEngineCommon

## Field-application helpers shared by every concrete engine parser (Diesel/DieselElectric/
## ElectricSeries/ElectricInduction) - NOT a section parser itself (no parse()/prefix), just
## the common part of Engine:'s and Cntrl.'s field sets, factored out because every concrete
## TrainEngine subclass inherits these Godot properties. LoadFIZ_Engine common subset:
## Mover.cpp:11119; Cntrl. engine subset: Mover.cpp:10707.


## EngineType= decode (TrainEngine.EngineType - the enum this class family owns).
## LoadFIZ_EngineDecode: Mover.cpp:11689. Used by Engine: and by other sections that reference
## an engine type (Light:/Clima: generator engine).
static func parse_engine_type(value: String, default_value: int = TrainEngine.NONE) -> int:
    if value.is_empty():
        return default_value
    match value.to_lower():
        "electricseriesmotor": return TrainEngine.ELECTRIC_SERIES_MOTOR
        "dieselengine": return TrainEngine.DIESEL
        "steamengine": return TrainEngine.STEAM
        "wheelsdriven": return TrainEngine.WHEELS_DRIVEN
        "dumb": return TrainEngine.DUMB
        "dieselelectric", "dumbde": return TrainEngine.DIESEL_ELECTRIC
        "electricinductionmotor": return TrainEngine.ELECTRIC_INDUCTION_MOTOR
        "main": return TrainEngine.MAIN
        _: return TrainEngine.NONE


## Engine: fields common to every EngineType (Trans=, TransEff, motor blowers, ...).
static func apply_engine_common(node: TrainEngine, kv: Dictionary, context: FizImportContext) -> void:
    if kv.has("Trans"):
        var parts: PackedStringArray = FizLineUtil.get_string(kv, "Trans").split(":")
        if parts.size() == 2:
            node.transmission_gear_teeth_motor = parts[0].to_int()
            node.transmission_gear_teeth_wheel = parts[1].to_int()
    if kv.has("TransEff"):
        node.transmission_efficiency = FizLineUtil.get_float(kv, "TransEff")
    if kv.has("Ftmax"):
        node.maximum_traction_force = FizLineUtil.get_float(kv, "Ftmax")
    if kv.has("MotorBlowersSpeed"):
        node.motor_blowers_speed = FizLineUtil.get_float(kv, "MotorBlowersSpeed")
    if kv.has("MotorBlowersSustainTime"):
        node.motor_blowers_sustain_time = FizLineUtil.get_float(kv, "MotorBlowersSustainTime")
    if kv.has("MotorBlowersStartVelocity"):
        node.motor_blowers_start_velocity = FizLineUtil.get_float(kv, "MotorBlowersStartVelocity")
    if kv.has("InvNo"):
        node.inverters_count = FizLineUtil.get_int(kv, "InvNo")

    # PressureSwitch's absent-key default (true, unless the vehicle is EZT) differs from the
    # compiled default (false).
    var pressure_switch_default: bool = context.train_type != TrainController.TRAIN_TYPE_EZT
    node.pressure_switch_present = FizLineUtil.get_bool(kv, "PressureSwitch", pressure_switch_default)


## The controller-position-count subset of Cntrl. (stashed on context.cntrl_kv by
## FizTrainCntrlParser, since Cntrl. conventionally precedes Engine: in real files).
static func apply_cntrl_engine_subset(node: TrainEngine, cntrl_kv: Dictionary) -> void:
    if cntrl_kv.is_empty():
        return
    if cntrl_kv.has("MCPN"):
        node.cntrl_main_controller_position_count = FizLineUtil.get_int(cntrl_kv, "MCPN")
    if cntrl_kv.has("SCPN"):
        node.cntrl_shunt_controller_position_count = FizLineUtil.get_int(cntrl_kv, "SCPN")
    if cntrl_kv.has("DirChangeMaxPos"):
        node.cntrl_direction_change_max_position = FizLineUtil.get_int(cntrl_kv, "DirChangeMaxPos")
    if cntrl_kv.has("CoupledCtrl"):
        node.cntrl_coupled_controllers = FizLineUtil.get_bool(cntrl_kv, "CoupledCtrl")
    if cntrl_kv.has("Camshaft"):
        node.cntrl_has_camshaft = FizLineUtil.get_bool(cntrl_kv, "Camshaft")
    if cntrl_kv.has("ScndS"):
        node.cntrl_series_shunt_on_series_position = FizLineUtil.get_bool(cntrl_kv, "ScndS")
    if cntrl_kv.has("IniCDelay"):
        node.cntrl_initial_controller_delay = FizLineUtil.get_float(cntrl_kv, "IniCDelay")
    if cntrl_kv.has("SCDelay"):
        node.cntrl_controller_step_delay = FizLineUtil.get_float(cntrl_kv, "SCDelay")
    # SCDDelay's absent-key default (== SCDelay) differs from the compiled default (0.0).
    node.cntrl_controller_step_down_delay = FizLineUtil.get_float(
            cntrl_kv, "SCDDelay", FizLineUtil.get_float(cntrl_kv, "SCDelay"))
    if cntrl_kv.has("FSCircuit"):
        node.cntrl_fast_series_circuit = FizLineUtil.get_bool(cntrl_kv, "FSCircuit")
    if cntrl_kv.has("EIMCtrlAddZeros"):
        node.cntrl_eim_control_additional_zeros = FizLineUtil.get_bool(cntrl_kv, "EIMCtrlAddZeros")
    if cntrl_kv.has("EIMCtrlEmergency"):
        node.cntrl_eim_control_emergency = FizLineUtil.get_bool(cntrl_kv, "EIMCtrlEmergency")
    if cntrl_kv.has("EIMCtrlType"):
        node.cntrl_eim_control_type = clampi(FizLineUtil.get_int(cntrl_kv, "EIMCtrlType"), 0, 3)
    if cntrl_kv.has("MotorBlowersStart"):
        node.motor_blowers_start_mode = FizTrainControllerParser.parse_start_mode(
                FizLineUtil.get_string(cntrl_kv, "MotorBlowersStart"), TrainEngine.START_MODE_MANUAL)
    if node is TrainDieselEngine and cntrl_kv.has("OilStart"):
        (node as TrainDieselEngine).oil_pump_start_mode = FizTrainControllerParser.parse_start_mode(
                        FizLineUtil.get_string(cntrl_kv, "OilStart"), TrainEngine.START_MODE_MANUAL)

    match FizLineUtil.get_string(cntrl_kv, "AutoRelay").to_lower():
        "optional": node.cntrl_auto_relay_mode = TrainEngine.AUTO_RELAY_OPTIONAL
        "yes": node.cntrl_auto_relay_mode = TrainEngine.AUTO_RELAY_YES


## Shared MotorParamTable0:/MotorParamTable: row parser - both sections share the same idx+
## mfi+mIsat+fi+Isat row shape and populate the same TrainEngine.motor_param_table
## (MotorParameter resources), which TrainEngine::_do_update_internal_mover already pushes into
## the mover's MotorParam[] for every engine type (not just ElectricSeriesMotor - DieselElectric's
## TractionForce() reads the same MotorParam[] table for its traction motor characteristics).
## Column order verified directly against the two real readers, not the wiki (which marks every
## column "?"): readMPTElectricSeries (Mover.cpp:9004) reads idx, mfi, mIsat, fi, Isat, then an
## OPTIONAL 5th value as an auto-shunt flag (int==1) - any further column is real-file noise the
## original parser never touches (e.g. 303e-ep.fiz's MotorParamTable0 rows carry a trailing
## duplicate of mIsat/Isat that both engines ignore). readMPTDieselElectric (Mover.cpp:9028)
## reads the same first four fields, then two REQUIRED trailing columns as MPTRelay[]'s
## shunting_up/shunting_down thresholds instead of an auto-shunt flag - pass
## p_is_diesel_electric to select that variant.
static func parse_motor_param_row(p: MaszynaParser, p_is_diesel_electric: bool = false) -> MotorParameter:
    var tokens: Array = p.get_tokens(7)
    var min_tokens: int = (7 if p_is_diesel_electric else 5)
    if tokens.size() < min_tokens:
        return null
    var item := MotorParameter.new()
    item.voltage_constant_multiplier = float(tokens[1])   # mfi
    item.saturation_current_multiplier = float(tokens[2]) # mIsat
    item.voltage_constant = float(tokens[3])               # fi
    item.saturation_current = float(tokens[4])             # Isat
    if p_is_diesel_electric:
        item.shunting_up = float(tokens[5])
        item.shunting_down = float(tokens[6])
    elif tokens.size() >= 6:
        item.auto_switch = (int(tokens[5]) == 1)
    return item


## Power:'s fields, common to the whole TrainElectricEngine family (Series + Induction).
## Stashed on context.power_kv by FizTrainPowerParser. LoadFIZ_Power: Mover.cpp:11058,
## LoadFIZ_PowerParamsDecode (CurrentCollector case): Mover.cpp:11547.
static func apply_power(node: TrainElectricEngine, power_kv: Dictionary) -> void:
    if power_kv.is_empty():
        return
    if power_kv.has("EnginePower"):
        node.power_source = FizTrainControllerParser.parse_power_source(FizLineUtil.get_string(power_kv, "EnginePower"))
    if power_kv.has("CollectorsNo"):
        node.power_current_collector_number_of_collectors = FizLineUtil.get_int(power_kv, "CollectorsNo")
    if power_kv.has("MinH"):
        node.power_current_collector_min_collector_lifting = FizLineUtil.get_float(power_kv, "MinH")
    if power_kv.has("MaxH"):
        node.power_current_collector_max_collector_lifting = FizLineUtil.get_float(power_kv, "MaxH")
    if power_kv.has("CSW"):
        node.power_current_collector_sliding_width = FizLineUtil.get_float(power_kv, "CSW")
    if power_kv.has("PhysicalLayout"):
        node.power_current_collector_physical_layout = FizLineUtil.get_int(power_kv, "PhysicalLayout", 3)
    if power_kv.has("OverVoltProt"):
        node.power_current_collector_overvoltage_relay = FizLineUtil.get_bool(power_kv, "OverVoltProt")
    if power_kv.has("TransducerInputV"):
        node.power_transducer_input_voltage = FizLineUtil.get_float(power_kv, "TransducerInputV")
    if power_kv.has("PowerTrans"):
        node.power_cable_source = FizTrainControllerParser.parse_power_type(FizLineUtil.get_string(power_kv, "PowerTrans"))
    if power_kv.has("SteamPress"):
        node.power_cable_steam_pressure = FizLineUtil.get_float(power_kv, "SteamPress")

    var max_voltage: float = FizLineUtil.get_float(power_kv, "MaxVoltage")
    if power_kv.has("MaxVoltage"):
        node.power_current_collector_max_voltage = max_voltage
    if power_kv.has("MaxCurrent"):
        node.power_current_collector_max_current = FizLineUtil.get_float(power_kv, "MaxCurrent")
    # MinV/InsetV's absent-key defaults (fractions of MaxVoltage) differ from the compiled
    # defaults (0.0) whenever MaxVoltage is set.
    node.power_current_collector_min_main_switch_voltage = FizLineUtil.get_float(power_kv, "MinV", 0.5 * max_voltage)
    node.power_current_collector_required_main_switch_voltage = FizLineUtil.get_float(power_kv, "InsetV", 0.6 * max_voltage)
    node.power_current_collector_min_pantograph_tank_pressure = FizLineUtil.get_float(power_kv, "MinPress", 3.5)
    node.power_current_collector_max_pantograph_tank_pressure = FizLineUtil.get_float(power_kv, "MaxPress", 5.0)
