@tool
extends RefCounted
class_name FizTrainBrakeParser

## Brake: section, the brake-relevant subset of Cntrl. (delegated from FizTrainCntrlParser),
## and the brake-position table (BPT, rows immediately following the Cntrl. line) and
## CompressorList: table -> VehicleBrake. LoadFIZ_Brake: Mover.cpp:10394, brake subset of
## LoadFIZ_Cntrl: Mover.cpp:10707, readBPT: Mover.cpp:9200, readCompressorList: Mover.cpp:9499.
##
## Setters are only called when the corresponding FIZ key is present, except where the
## original LoadFIZ_Brake/_Cntrl logic computes a default that genuinely differs from
## VehicleBrake's compiled default (rig_effectiveness, valve/type "ESt" fallback).

const _VALVE_MAP := {
    "w": VehicleBrake.BRAKE_VALVE_W, "w_lu_l": VehicleBrake.BRAKE_VALVE_W_LU_L,
    "w_lu_xr": VehicleBrake.BRAKE_VALVE_W_LU_XR, "w_lu_vi": VehicleBrake.BRAKE_VALVE_W_LU_VI,
    "k": VehicleBrake.BRAKE_VALVE_K, "kg": VehicleBrake.BRAKE_VALVE_KG, "kp": VehicleBrake.BRAKE_VALVE_KP,
    "kss": VehicleBrake.BRAKE_VALVE_KSS, "kkg": VehicleBrake.BRAKE_VALVE_KKG, "kkp": VehicleBrake.BRAKE_VALVE_KKP,
    "kks": VehicleBrake.BRAKE_VALVE_KKS, "hikp1": VehicleBrake.BRAKE_VALVE_HIKP1,
    "hikss": VehicleBrake.BRAKE_VALVE_HIKSS, "hikg1": VehicleBrake.BRAKE_VALVE_HIKG1,
    "ke": VehicleBrake.BRAKE_VALVE_KE, "sw": VehicleBrake.BRAKE_VALVE_SW, "ested": VehicleBrake.BRAKE_VALVE_ESTED,
    "nest3": VehicleBrake.BRAKE_VALVE_NEST3, "est3": VehicleBrake.BRAKE_VALVE_EST3, "lst": VehicleBrake.BRAKE_VALVE_LST,
    "est4": VehicleBrake.BRAKE_VALVE_EST4, "est3al2": VehicleBrake.BRAKE_VALVE_EST3AL2,
    "ep1": VehicleBrake.BRAKE_VALVE_EP1, "ep2": VehicleBrake.BRAKE_VALVE_EP2, "m483": VehicleBrake.BRAKE_VALVE_M483,
    "cv1_l_tr": VehicleBrake.BRAKE_VALVE_CV1_L_TR, "cv1": VehicleBrake.BRAKE_VALVE_CV1,
    "cv1_r": VehicleBrake.BRAKE_VALVE_CV1_R,
}

const _METHOD_MAP := {
    "p10-bg": VehicleBrake.BRAKE_METHOD_P10_BG, "p10-bgu": VehicleBrake.BRAKE_METHOD_P10_BGU,
    "fr513": VehicleBrake.BRAKE_METHOD_FR513, "cosid": VehicleBrake.BRAKE_METHOD_COSID,
    "p10ybg": VehicleBrake.BRAKE_METHOD_P10Y_BG, "p10ybgu": VehicleBrake.BRAKE_METHOD_P10Y_BGU,
    "disk1": VehicleBrake.BRAKE_METHOD_D1, "disk1+mg": VehicleBrake.BRAKE_METHOD_D1MG,
    "disk2": VehicleBrake.BRAKE_METHOD_D2,
}

const _HANDLE_TYPE_MAP := {
    "fv4a": VehicleBrake.BRAKE_HANDLE_TYPE_FV4A, "test": VehicleBrake.BRAKE_HANDLE_TYPE_TESTH,
    "d2": VehicleBrake.BRAKE_HANDLE_TYPE_D2, "mhz_en57": VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_EN57,
    "mhz_k5p": VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_K5P, "mhz_k8p": VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_K8P,
    "mhz_6p": VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_6P, "m394": VehicleBrake.BRAKE_HANDLE_TYPE_M394,
    "knorr": VehicleBrake.BRAKE_HANDLE_TYPE_KNORR, "west": VehicleBrake.BRAKE_HANDLE_TYPE_WESTINGHOUSE,
    "fvel6": VehicleBrake.BRAKE_HANDLE_TYPE_FVEL6, "fve408": VehicleBrake.BRAKE_HANDLE_TYPE_FVE408,
    "st113": VehicleBrake.BRAKE_HANDLE_TYPE_ST113,
}

const _LOCAL_BRAKE_TYPE_MAP := {
    "manualbrake": VehicleBrake.LOCAL_BRAKE_TYPE_MANUAL, "pneumaticbrake": VehicleBrake.LOCAL_BRAKE_TYPE_PNEUMATIC,
    "hydraulicbrake": VehicleBrake.LOCAL_BRAKE_TYPE_HYDRAULIC,
}


func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    if prefix == "CompressorList:":
        # header carries only CompressorListPosNo/Wrap/DefPos, none of which have a Godot
        # property home on VehicleBrake yet - just start collecting rows.
        _active_table = "CompressorList"
        _compressor_rows = []
        return

    var node := VehicleBrake.new()
    _parse_brake(kv, node)
    context.add_part("VehicleBrake", node)


func _parse_brake(kv: Dictionary, node: VehicleBrake) -> void:
    if kv.has("AirLeakRate"):
        node.air_leak_multiplier = FizLineUtil.get_float(kv, "AirLeakRate") * 0.01

    var method_str: String = FizLineUtil.get_string(kv, "BM").to_lower()
    if method_str:
        if _METHOD_MAP.has(method_str):
            node.brake_method = _METHOD_MAP[method_str]
        else:
            push_warning("FIZ Brake:BM: unmapped value '%s'" % method_str)

    if kv.has("MBF"):
        node.brake_force_max = FizLineUtil.get_float(kv, "MBF")
    if kv.has("TBF"):
        node.brake_force_traction = FizLineUtil.get_float(kv, "TBF")

    if kv.has("MaxBP"):
        var max_cylinder_pressure: float = FizLineUtil.get_float(kv, "MaxBP")
        node.max_cylinder_pressure = max_cylinder_pressure
        if kv.has("BCN"):
            node.cylinder_count = FizLineUtil.get_int(kv, "BCN")
        node.max_aux_pressure = FizLineUtil.get_float(kv, "MaxLBP", max_cylinder_pressure)
        if kv.has("TareMaxBP"):
            node.max_tare_pressure = FizLineUtil.get_float(kv, "TareMaxBP")
        if kv.has("MedMaxBP"):
            node.max_medium_pressure = FizLineUtil.get_float(kv, "MedMaxBP")
        if kv.has("MaxASBP"):
            node.max_antislip_pressure = FizLineUtil.get_float(kv, "MaxASBP")

    if kv.has("BCR"):
        node.cylinder_radius = FizLineUtil.get_float(kv, "BCR")
    if kv.has("BCD"):
        node.cylinder_distance = FizLineUtil.get_float(kv, "BCD")
    if kv.has("BCS"):
        node.cylinder_spring_force = FizLineUtil.get_float(kv, "BCS")
    if kv.has("BSA"):
        node.piston_stroke_adjuster_resistance = FizLineUtil.get_float(kv, "BSA")
    # rig_effectiveness' FIZ-format default (1.0) differs from VehicleBrake's compiled default (0.0).
    node.rig_effectiveness = FizLineUtil.get_float(kv, "BRE", 1.0)
    if kv.has("BCM"):
        node.cylinder_gear_ratio = FizLineUtil.get_float(kv, "BCM")
    if kv.has("BCMlo"):
        node.cylinder_gear_ratio_low = FizLineUtil.get_float(kv, "BCMlo")
    if kv.has("BCMHi"):
        node.cylinder_gear_ratio_high = FizLineUtil.get_float(kv, "BCMHi")
    if kv.has("Size"):
        node.est_valve_size = FizLineUtil.get_int(kv, "Size")
    if kv.has("NBpA"):
        node.friction_elements_per_axle = FizLineUtil.get_int(kv, "NBpA")

    if kv.has("LPOn"):
        node.main_pipe_blocking_pressure = FizLineUtil.get_float(kv, "LPOn")
    if kv.has("LPOff"):
        node.main_pipe_unblocking_pressure = FizLineUtil.get_float(kv, "LPOff")
    if kv.has("HandlePipeUnlockPos"):
        node.main_pipe_minimum_unblocking_handle_position = FizLineUtil.get_int(kv, "HandlePipeUnlockPos")
    if kv.has("EmergencyCutsOffHandle"):
        node.main_pipe_emergency_cuts_off_handle = FizLineUtil.get_bool(kv, "EmergencyCutsOffHandle")

    var high_pressure: float = FizLineUtil.get_float(kv, "HiPP", 5.0)
    if kv.has("HiPP"):
        node.pipe_pressure_max = high_pressure
    node.pipe_pressure_min = FizLineUtil.get_float(kv, "LoPP", minf(high_pressure, 3.5))

    if kv.has("Vv"):
        node.tank_volume_main = FizLineUtil.get_float(kv, "Vv")
    if kv.has("BVV"):
        node.tank_volume_aux = FizLineUtil.get_float(kv, "BVV")

    if kv.has("MinCP"):
        node.compressor_cab_a_min_pressure = FizLineUtil.get_float(kv, "MinCP")
    if kv.has("MaxCP"):
        node.compressor_cab_a_max_pressure = FizLineUtil.get_float(kv, "MaxCP")
    if kv.has("MinCP_B"):
        node.compressor_cab_b_min_pressure = FizLineUtil.get_float(kv, "MinCP_B")
    if kv.has("MaxCP_B"):
        node.compressor_cab_b_max_pressure = FizLineUtil.get_float(kv, "MaxCP_B")
    if kv.has("CompressorSpeed"):
        node.compressor_speed = FizLineUtil.get_float(kv, "CompressorSpeed")
    if kv.has("CompressorPower"):
        match FizLineUtil.get_string(kv, "CompressorPower").to_lower():
            "main": node.compressor_power = VehicleBrake.COMPRESSOR_POWER_MAIN
            "converter": node.compressor_power = VehicleBrake.COMPRESSOR_POWER_CONVERTER
            "engine": node.compressor_power = VehicleBrake.COMPRESSOR_POWER_ENGINE
            "coupler1": node.compressor_power = VehicleBrake.COMPRESSOR_POWER_COUPLER1
            "coupler2": node.compressor_power = VehicleBrake.COMPRESSOR_POWER_COUPLER2
    if kv.has("CompressorTankValve"):
        node.compressor_tank_valve_active = FizLineUtil.get_bool(kv, "CompressorTankValve")
    if kv.has("EVArea"):
        node.compressor_emergency_valve_area = FizLineUtil.get_float(kv, "EVArea")
    if kv.has("MinEVP"):
        node.compressor_lower_emergency_closing_pressure = FizLineUtil.get_float(kv, "MinEVP")
    if kv.has("MaxEVP"):
        node.compressor_higher_emergency_closing_pressure = FizLineUtil.get_float(kv, "MaxEVP")

    if kv.has("UBB1"):
        node.universal_brake_button_1 = FizLineUtil.get_int(kv, "UBB1")
    if kv.has("UBB2"):
        node.universal_brake_button_2 = FizLineUtil.get_int(kv, "UBB2")
    if kv.has("UBB3"):
        node.universal_brake_button_3 = FizLineUtil.get_int(kv, "UBB3")

    if kv.has("RM"):
        node.rapid_transfer = FizLineUtil.get_float(kv, "RM")
    if kv.has("RV"):
        node.rapid_switching_speed = FizLineUtil.get_float(kv, "RV")

    var valve_str: String = FizLineUtil.get_string(kv, "BrakeValve").to_lower()
    if valve_str:
        if _VALVE_MAP.has(valve_str):
            node.valve_type = _VALVE_MAP[valve_str]
        elif valve_str.find("est") != -1:
            node.valve_type = VehicleBrake.BRAKE_VALVE_EST3
        else:
            node.valve_type = VehicleBrake.BRAKE_VALVE_OTHER


## Called by FizTrainCntrlParser with the full Cntrl. key/value set - applies only the
## brake-relevant subset.
func apply_cntrl(kv: Dictionary, node: VehicleBrake, context: FizImportContext) -> void:
    var brake_system: int = VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL
    match FizLineUtil.get_string(kv, "BrakeSystem").to_lower():
        "pneumatic": brake_system = VehicleBrake.BRAKE_SYSTEM_PNEUMATIC
        "electropneumatic": brake_system = VehicleBrake.BRAKE_SYSTEM_ELECTRO_PNEUMATIC
    node.cntrl_brake_system = brake_system
    context.brake_system = brake_system

    if brake_system == VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL:
        return

    if kv.has("BCPN"):
        node.cntrl_brake_ctrl_position_count = FizLineUtil.get_int(kv, "BCPN")
    if kv.has("BDelay1"):
        node.cntrl_brake_delay_1 = FizLineUtil.get_float(kv, "BDelay1")
    if kv.has("BDelay2"):
        node.cntrl_brake_delay_2 = FizLineUtil.get_float(kv, "BDelay2")
    if kv.has("BDelay3"):
        node.cntrl_brake_delay_3 = FizLineUtil.get_float(kv, "BDelay3")
    if kv.has("BDelay4"):
        node.cntrl_brake_delay_4 = FizLineUtil.get_float(kv, "BDelay4")

    var delays_str: String = FizLineUtil.get_string(kv, "BrakeDelays").to_lower()
    const _DELAY_MAP := {"g": 1, "p": 2, "r": 4, "gp": 3, "pr": 6, "gpr": 7, "gpr+mg": 15, "pr+mg": 14}
    if _DELAY_MAP.has(delays_str):
        node.cntrl_brake_delays = _DELAY_MAP[delays_str]

    var op_modes_str: String = FizLineUtil.get_string(kv, "BrakeOpModes").to_lower()
    match op_modes_str:
        "pn": node.cntrl_brake_op_modes = VehicleBrake.BRAKE_OP_MODE_PN
        "pnepmed": node.cntrl_brake_op_modes = VehicleBrake.BRAKE_OP_MODE_PNEPMED
        "pnep": pass # TODO: exact bitmask for the intermediate PN+EP-only mode is unverified.

    var handle_str: String = FizLineUtil.get_string(kv, "BrakeHandle").to_lower()
    if _HANDLE_TYPE_MAP.has(handle_str):
        node.cntrl_brake_handle_type = _HANDLE_TYPE_MAP[handle_str]
    var loc_handle_str: String = FizLineUtil.get_string(kv, "LocBrakeHandle").to_lower()
    if _HANDLE_TYPE_MAP.has(loc_handle_str):
        node.cntrl_local_brake_handle_type = _HANDLE_TYPE_MAP[loc_handle_str]

    var local_brake_str: String = FizLineUtil.get_string(kv, "LocalBrake").to_lower()
    if _LOCAL_BRAKE_TYPE_MAP.has(local_brake_str):
        node.cntrl_local_brake_type = _LOCAL_BRAKE_TYPE_MAP[local_brake_str]
    if kv.has("ManualBrake"):
        node.cntrl_manual_brake_present = FizLineUtil.get_bool(kv, "ManualBrake")

    match FizLineUtil.get_string(kv, "ASB").to_lower():
        "manual": node.cntrl_anti_skid_brake_type = VehicleBrake.ANTI_SKID_BRAKE_MANUAL
        "automatic": node.cntrl_anti_skid_brake_type = VehicleBrake.ANTI_SKID_BRAKE_AUTOMATIC
        "yes": node.cntrl_anti_skid_brake_type = VehicleBrake.ANTI_SKID_BRAKE_AUTOMATIC

    var dynamic_str: String = FizLineUtil.get_string(kv, "DynamicBrake").to_lower()
    match dynamic_str:
        "passive": node.cntrl_dynamic_brake_type = VehicleBrake.DYNAMIC_BRAKE_PASSIVE
        "switch": node.cntrl_dynamic_brake_type = VehicleBrake.DYNAMIC_BRAKE_SWITCH
        "reversal": node.cntrl_dynamic_brake_type = VehicleBrake.DYNAMIC_BRAKE_REVERSAL
        "automatic": node.cntrl_dynamic_brake_type = VehicleBrake.DYNAMIC_BRAKE_AUTOMATIC

    if kv.has("LocalBrakeTraxx"):
        node.cntrl_local_brake_traxx = FizLineUtil.get_bool(kv, "LocalBrakeTraxx")
    if kv.has("ReleaseParkingBySpringBrake"):
        node.cntrl_release_parking_by_spring_brake = FizLineUtil.get_bool(kv, "ReleaseParkingBySpringBrake")
    if kv.has("ReleaseParkingBySpringBrakeWhenDoorIsOpen"):
        node.cntrl_release_parking_by_spring_brake_when_door_open = FizLineUtil.get_bool(kv, "ReleaseParkingBySpringBrakeWhenDoorIsOpen")
    if kv.has("SpringBrakeCutsOffDrive"):
        node.cntrl_spring_brake_cuts_off_drive = FizLineUtil.get_bool(kv, "SpringBrakeCutsOffDrive")
    if kv.has("SpringBrakeDriveEmergencyVel"):
        node.cntrl_spring_brake_drive_emergency_velocity = FizLineUtil.get_float(kv, "SpringBrakeDriveEmergencyVel")


func wants_bpt_table(context: FizImportContext) -> bool:
    if context.brake_system == VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL:
        return false
    _active_table = "BPT"
    _bpt_rows = []
    return true


var _bpt_rows: Array[BrakePressureTableItem] = []
var _compressor_rows: Array[CompressorListItem] = []
var _active_table: String = ""


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    match _active_table:
        "BPT": _parse_bpt_row(p)
        "CompressorList": _parse_compressor_row(p)


func _parse_bpt_row(p: MaszynaParser) -> void:
    var tokens: Array = p.get_tokens(5)
    if tokens.size() < 5:
        return
    var item := BrakePressureTableItem.new()
    item.handle_position = int(tokens[0])
    item.pipe_pressure = float(tokens[1])
    item.brake_cylinder_pressure = float(tokens[2])
    item.fill_speed = float(tokens[3])
    match String(tokens[4]).to_lower():
        "pneumatic", "p": item.brake_type = BrakePressureTableItem.BRAKE_TYPE_PNEUMATIC
        "electropneumatic", "ep": item.brake_type = BrakePressureTableItem.BRAKE_TYPE_ELECTRO_PNEUMATIC
        _: item.brake_type = BrakePressureTableItem.BRAKE_TYPE_INDIVIDUAL
    _bpt_rows.append(item)


func _parse_compressor_row(p: MaszynaParser) -> void:
    var tokens: Array = p.get_tokens(4)
    if tokens.size() < 4:
        return
    var item := CompressorListItem.new()
    item.allow = int(tokens[0])
    item.speed_factor = int(tokens[1])
    item.min_pressure_factor = int(tokens[2])
    item.max_pressure_factor = int(tokens[3])
    _compressor_rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node: VehicleBrake = context.get_part("VehicleBrake")
    if node == null:
        return
    if _bpt_rows:
        node.brake_pressure_table = _bpt_rows
        _bpt_rows = []
    if _compressor_rows:
        node.compressor_list = _compressor_rows
        _compressor_rows = []
    _active_table = ""
