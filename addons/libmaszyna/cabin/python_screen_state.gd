extends RefCounted
class_name PythonScreenState

## The dictionary a Python cab screen is drawn from - the original's TTrain::GetTrainState()
## (Train.cpp:696-940), under its own key names and value types.
##
## A script reads its keys with state['key'] and a missing one raises KeyError, which stops the
## whole screen from drawing. So every key the original hands over is always present: the ones the
## vehicle's state has an equivalent for come from it, the rest carry the neutral value of their
## type (see TODO.md for what is not ported yet).

## Train.cpp:924 fEIMParams[9][10] - row 0 is the train, rows 1-8 the powered cars
const EIM_CAR_COUNT:int = 8
## Train.cpp:921 fPress[20][7], and the 20 cars of the door/name/slip tables
const CAR_COUNT:int = 20
## Train.cpp:637 ggUniversals
const UNIVERSAL_COUNT:int = 30
## Train.cpp:809-813
const EIM_TRAIN_FIELDS:Array[String] = ["fd", "fdt", "fdb", "pd", "pdt", "pdb", "itothv", "1", "2", "3"]
const EIM_CAR_FIELDS:Array[String] = ["fr", "frt", "frb", "pr", "prt", "prb", "im", "vm", "ihv", "uhv"]
const DIESEL_FIELDS:Array[String] = [
    "enrot", "nrot", "fill_des", "fill_real", "clutch_des", "clutch_real", "water_temp", "oil_press",
    "engine_temp", "retarder_fill"]
const PRESSURE_FIELDS:Array[String] = ["bc", "bp", "sp", "cp", "rp", "mass", "spring"]

## Original key -> key of the occupied vehicle's state holding the same Mover field
## (Train.cpp:711-805; every pair checked against the getter behind the state key)
const STATE_KEYS:Dictionary[String, String] = {
    "cab": "cabin_occupied",                        # CabOccupied
    "cabactive": "cabin",                           # CabActive
    "battery": "power24_available",                 # Power24vIsAvailable
    "converter": "power110_available",              # Power110vIsAvailable
    "direction": "direction",                       # DirActive
    "speedctrl": "speed_control/selected_velocity", # SpeedCtrlValue
    "speedctrlpower": "speed_control/desired_power", # SpeedCtrlUnit.DesiredPower
    "speedctrlactive": "speed_control/active",      # SpeedCtrlUnit.IsActive
    "emergency_brake": "alarm_chain_pulled",        # AlarmChainFlag
    "ca": "vigilance_blinking",                     # SecuritySystem.is_vigilance_blinking()
    "shp": "cabsignal_blinking",                    # SecuritySystem.is_cabsignal_blinking()
    "radio": "radio_enabled",                       # Radio
    "radio_channel": "radio_channel",
    "radio_volume": "radio_volume",                 # TTrain::m_radiovolume
    "distance_counter": "distance_counter",         # TTrain::m_distancecounter
    "pipelock": "main_pipe_locked",                 # LockPipe
    "door_lock": "doors_lock_enabled",              # Doors.lock_enabled
    "door_step": "doors_step_enabled",              # Doors.step_enabled
    "door_permit_left": "doors_left_open_permit",   # Doors.instances[left].open_permit
    "door_permit_right": "doors_right_open_permit", # Doors.instances[right].open_permit
    "slipping_wheels": "slipping_wheels",           # SlippingWheels
    "sanding": "sand_active",                       # SandDose
    "odometer": "total_distance",                   # DistCounter
    "epfuse": "dcemued/ep_fuse",                    # EpFuse
}
## Original key -> key of the controlled vehicle's state (mvControlled - the powered car of a
## multiple unit, TDynamicObject::FindPowered(), DynObj.cpp:7772)
const CONTROLLED_STATE_KEYS:Dictionary[String, String] = {
    "linebreaker": "main_switch_enabled",           # Mains
    "converter_overload": "converter_overload",     # ConvOvldFlag
    "compress": "compressor_enabled",               # CompressorFlag
    "mainctrl_pos": "controller_main_position",     # MainCtrlPos
    "main_ctrl_actual_pos": "controller_main_actual_position", # MainCtrlActualPos
    "scndctrl_pos": "controller_second_position",   # ScndCtrlPos
    "brakectrl_pos": "brake_controller_position",   # fBrakeCtrlPos
    "localbrake_pos": "brake_local_position_normalized", # LocalBrakePosA
    "fuse": "fuse_active",                          # FuseFlag
}
## TMoverParameters::light bits (MOVER.h:188) of the lamps the state carries
const LIGHT_BITS:Dictionary[String, int] = {
    "headlight_left_enabled": 1 << 0,
    "redmarker_left_enabled": 1 << 1,
    "headlight_upper_enabled": 1 << 2,
    "headlight_right_enabled": 1 << 4,
    "redmarker_right_enabled": 1 << 5,
}
## Train.cpp:765 dir_brake / :772 indir_brake - a control pressure that counts as braking
const BRAKE_PRESSURE_THRESHOLD:float = 0.2
## TDynamicObject::FindPowered() - a vehicle with more power than this drives
const POWERED_THRESHOLD:float = 1.0
## Train.cpp:8699 - a compressor that turns
const COMPRESSOR_SPEED_THRESHOLD:float = 0.00001
const SECONDS_PER_HOUR:int = 3600
const SECONDS_PER_MINUTE:int = 60
## A pantograph carrier publishes its collector (EnginePowerSource.SourceType == CurrentCollector)
const COLLECTOR_KEY:String = "current_collector/pantograph_first_active"

static var _neutral_state:Dictionary = {}


## `parameters` are the screen's own `parameters:` from the MMD (Train.cpp:706)
static func compose(train_id:String, parameters:Dictionary) -> Dictionary:
    var result:Dictionary = _neutral_state.duplicate()
    result.merge(parameters, true)
    var vehicle:RID = CabinSystem.vehicle_rid(train_id)
    if not vehicle.is_valid():
        return result
    var state:Dictionary = CabinSystem.vehicle_state(train_id)
    var config:Dictionary = CabinSystem.vehicle_config(train_id)

    # the vehicles of this unit and of everything under its control, in the order find_vehicle()
    # searches them: this one, then towards the rear, then towards the front (DynObj.h:889)
    var unit:Array[Dictionary] = _search_order(vehicle, VehicleController.COUPLING_ELEMENT_PERMANENT)
    var controlled_by:Array[Dictionary] = _search_order(vehicle, VehicleController.COUPLING_ELEMENT_CONTROL)
    var controlled:Dictionary = state
    # FindPowered() searches only the unit of an EZT/DMU; the train type is not in the config dump
    # (TODO.md), so every vehicle searches everything under its control
    for candidate:Dictionary in controlled_by:
        if candidate["config"].get("power", 0.0) > POWERED_THRESHOLD:
            controlled = candidate["state"]
            break
    var pantograph_unit:Dictionary = controlled
    for candidate:Dictionary in unit + controlled_by:
        if COLLECTOR_KEY in candidate["state"]:
            pantograph_unit = candidate["state"]
            break

    result["name"] = RailVehicleServer.vehicle_get_name(vehicle)   # DynamicObject->asName
    for key:String in STATE_KEYS:
        if STATE_KEYS[key] in state:
            result[key] = state[STATE_KEYS[key]]
    for key:String in CONTROLLED_STATE_KEYS:
        if CONTROLLED_STATE_KEYS[key] in controlled:
            result[key] = controlled[CONTROLLED_STATE_KEYS[key]]
    result["master"] = state.get("cabin_controleable", false)
    # Train.cpp:783-790 - the cab's generic toggles, with universal3 standing for the instrument light
    var cab_state:CabinState = CabinSystem.get_cabin_state(train_id, CabinSystem.occupied_cab(train_id))
    for index:int in UNIVERSAL_COUNT:
        result["universal%d" % index] = bool(cab_state.get_value(StringName("universal%d" % index), false))
    result["universal3"] = state.get("devices_light_enabled", false)   # InstrumentLightActive
    result["mainctrl_pos_count"] = config.get("main_controller_position_max", 0)   # MainCtrlPosNo
    result["velocity"] = absf(state.get("speed", 0.0))   # abs(Vel), km/h
    result["manual_brake"] = state.get("brake_manual_position", 0) > 0
    # the second half of the original's condition, the ED brake share (fEIMParams[0][5]), is not
    # in the state yet
    result["dir_brake"] = controlled.get("brake_control_pressure", 0.0) > BRAKE_PRESSURE_THRESHOLD
    # GetEDBCP() is 0 for every brake but TLSt and TEStED, which is the original's typeid test
    result["indir_brake"] = state.get("brake_edb_cylinder_pressure", 0.0) > BRAKE_PRESSURE_THRESHOLD
    result["pantpress"] = absf(pantograph_unit.get("current_collector/pantograph_tank_pressure", 0.0))
    result["traction_voltage"] = absf(pantograph_unit.get("current_collector/voltage", 0.0))
    for end:String in ["front", "rear"]:
        var bits:int = 0
        for lamp:String in LIGHT_BITS:
            if state.get("lights/%s_%s" % [end, lamp], false):
                bits |= LIGHT_BITS[lamp]
        result["lights_" + end] = bits

    # TTrain::Update(), Train.cpp:8644-8768 - the cars under control, from the end the occupied
    # cab faces (GetFirstDynamic(CabOccupied < 0 ? rear : front, control))
    var cab_end:int = 1 if state.get("cabin_occupied", 1) < 0 else 0
    var cars:Array = RailVehicleServer.vehicle_get_coupled(vehicle, cab_end, VehicleController.COUPLING_ELEMENT_CONTROL)
    var powered:int = 0
    var unit_number:int = 1
    var compressors:int = 0
    for index:int in mini(cars.size(), CAR_COUNT):
        var car:RID = cars[index]
        var car_state:Dictionary = RailVehicleServer.vehicle_dump_state(car)
        var car_number:int = index + 1
        result["eimp_pn%d_bc" % car_number] = car_state.get("brake_air_pressure", 0.0)   # BrakePress
        result["eimp_pn%d_bp" % car_number] = car_state.get("pipe_pressure", 0.0)   # PipePress
        result["eimp_pn%d_sp" % car_number] = car_state.get("feed_pipe_pressure", 0.0)   # ScndPipePress
        result["eimp_pn%d_spring" % car_number] = car_state.get("spring_brake/cylinder_pressure", 0.0)   # SpringBrake.SBP
        result["brakes_%d_spring_active" % car_number] = car_state.get("spring_brake/braking", false)   # IsActive
        result["brakes_%d_spring_shutoff" % car_number] = car_state.get("spring_brake/shut_off", false)   # ShuttOff
        var doors_left:bool = car_state.get("doors_left_position", 0.0) > 0.0
        var doors_right:bool = car_state.get("doors_right_position", 0.0) > 0.0
        result["doors_%d" % car_number] = doors_left or doors_right
        result["doors_l_%d" % car_number] = doors_left
        result["doors_r_%d" % car_number] = doors_right
        result["doorstep_l_%d" % car_number] = car_state.get("doors_left_step_position", 0.0) > 0.0
        result["doorstep_r_%d" % car_number] = car_state.get("doors_right_step_position", 0.0) > 0.0
        result["car_name%d" % car_number] = RailVehicleServer.vehicle_get_name(car)
        result["slip_%d" % car_number] = car_state.get("slipping_wheels", false)
        if unit_number <= EIM_CAR_COUNT:
            if COLLECTOR_KEY in car_state:
                result["eimp_u%d_pf" % unit_number] = result["eimp_u%d_pf" % unit_number] or car_state.get("current_collector/pantograph_first_active", false)
                result["eimp_u%d_pr" % unit_number] = result["eimp_u%d_pr" % unit_number] or car_state.get("current_collector/pantograph_second_active", false)
            # CompressorStart is never automatic here - the wrapper does not set it
            result["eimp_u%d_comp_a" % unit_number] = result["eimp_u%d_comp_a" % unit_number] or car_state.get("compressor_allowed", false)
        var brake:VehicleBrake = RailVehicleServer.vehicle_component_get(car, VehicleComponentType.COMPONENT_BRAKES) as VehicleBrake
        if brake and brake.compressor_speed > COMPRESSOR_SPEED_THRESHOLD:
            if unit_number <= EIM_CAR_COUNT:
                result["eimp_u%d_comp_w" % unit_number] = result["eimp_u%d_comp_w" % unit_number] or car_state.get("compressor_enabled", false)
            compressors += 1
            result["compressors_%d_allow" % compressors] = car_state.get("compressor_allowed", false)
            result["compressors_%d_work" % compressors] = car_state.get("compressor_enabled", false)
            result["compressors_%d_car_no" % compressors] = index
        var engine_type:int = car_state.get("engine_type", VehicleEngine.NONE)
        # eimc[eimc_p_Pmax] > 1 - an induction motor car; the diesels by their engine type
        if powered < EIM_CAR_COUNT and engine_type in [VehicleEngine.ELECTRIC_INDUCTION_MOTOR, VehicleEngine.DIESEL, VehicleEngine.DIESEL_ELECTRIC]:
            var powered_number:int = powered + 1
            if not engine_type == VehicleEngine.ELECTRIC_INDUCTION_MOTOR:
                result["diesel_param_%d_enrot" % powered_number] = car_state.get("engine_rpm_count", 0.0) * SECONDS_PER_MINUTE   # enrot * 60
                result["diesel_param_%d_nrot" % powered_number] = car_state.get("wheel_rotation_speed_rps", 0.0)   # nrot
                result["diesel_param_%d_fill_real" % powered_number] = car_state.get("diesel_fill", 0.0)   # dizel_fill
                result["diesel_param_%d_oil_press" % powered_number] = car_state.get("oil_pump_pressure", 0.0)   # OilPump.pressure
            result["eimp_c%d_ms" % powered_number] = car_state.get("main_switch_enabled", false)   # Mains
            result["eimp_c%d_cv" % powered_number] = car_state.get("battery_voltage", 0.0)   # BatteryVoltage
            result["eimp_c%d_fuse" % powered_number] = car_state.get("fuse_active", false)   # FuseFlag
            result["eimp_c%d_batt" % powered_number] = car_state.get("battery_enabled", false)   # Battery
            result["eimp_c%d_conv" % powered_number] = car_state.get("converter_enabled", false)   # ConverterFlag
            result["eimp_c%d_heat" % powered_number] = car_state.get("heating_enabled", false)   # Heating
            powered = powered_number
        # a control coupling that is not a permanent one ends a unit (Train.cpp:8757)
        if index + 1 < cars.size() and not cars[index + 1] in RailVehicleServer.vehicle_get_coupled(car, 0, VehicleController.COUPLING_ELEMENT_PERMANENT):
            unit_number += 1
    result["car_no"] = mini(cars.size(), CAR_COUNT)
    result["power_no"] = powered
    result["unit_no"] = unit_number
    result["compressors_no"] = compressors

    # world state data (Train.cpp:928-934)
    var seconds:int = int(MaszynaRuntime.time_of_day * SECONDS_PER_HOUR)
    result["hours"] = seconds / SECONDS_PER_HOUR
    result["minutes"] = seconds / SECONDS_PER_MINUTE % SECONDS_PER_MINUTE
    result["seconds"] = seconds % SECONDS_PER_MINUTE
    result["air_temperature"] = MaszynaRuntime.air_temperature
    result["light_level"] = MaszynaRuntime.light_level
    return result


## The state and config of every vehicle joined to this one by `element`, in the order
## TDynamicObject::find_vehicle() searches them: the vehicle itself, then towards its rear, then
## towards its front (DynObj.h:889)
static func _search_order(vehicle:RID, element:VehicleController.CouplingElement) -> Array[Dictionary]:
    var joined:Array = RailVehicleServer.vehicle_get_coupled(vehicle, 0, element)
    var own:int = joined.find(vehicle)
    var order:Array = [vehicle] + joined.slice(own + 1)
    for index:int in range(own - 1, -1, -1):
        order.append(joined[index])
    var result:Array[Dictionary] = []
    for joined_vehicle:RID in order:
        result.append({
            "state": RailVehicleServer.vehicle_dump_state(joined_vehicle),
            "config": RailVehicleServer.vehicle_dump_config(joined_vehicle),
        })
    return result


static func _static_init() -> void:
    var state:Dictionary = {
        "name": "", "cab": 0, "cabactive": 0, "master": false,
        "battery": false, "linebreaker": false, "main_init": false, "main_ready": false,
        "converter": false, "converter_overload": false, "compress": false, "pant_compressor": false,
        "lights_front": 0, "lights_rear": 0, "off_from_dimmer": false, "lights_compartments": false,
        "lights_train_front": 0, "lights_train_rear": 0,
        "direction": 0, "mainctrl_pos": 0, "mainctrl_pos_count": 0, "main_ctrl_actual_pos": 0,
        "scndctrl_pos": 0, "scnd_ctrl_actual_pos": 0, "brakectrl_pos": 0.0, "localbrake_pos": 0.0,
        "new_speed": 0.0, "speedctrl": 0.0, "speedctrlpower": 0.0, "speedctrlactive": false,
        "speedctrlstandby": false,
        "manual_brake": false, "dir_brake": false, "indir_brake": false, "emergency_brake": false,
        "brake_delay_flag": 0, "brake_op_mode_flag": 0, "pipelock": false,
        "ca": false, "shp": false, "distance_counter": 0.0, "pantpress": 0.0,
        "radio": false, "radio_channel": 0, "radio_volume": 0.0,
        "door_lock": false, "door_step": false, "door_permit_left": false, "door_permit_right": false,
        "velocity": 0.0, "tractionforce": 0.0, "slipping_wheels": false, "sanding": false,
        "odometer": 0.0,
        "traction_voltage": 0.0, "voltage": 0.0, "im": 0.0, "fuse": false, "epfuse": false,
        "power_drawn": 0.0, "power_returned": 0.0,
        "compressors_no": 0, "car_no": 0, "power_no": 0, "unit_no": 0,
        "velocity_desired": 0.0, "velroad": 0.0, "vellimitlast": 0.0, "velsignallast": 0.0,
        "velsignalnext": 0.0, "velnext": 0.0, "actualproximitydist": 0.0,
        # TTrainParameters::serialize() (mtable.cpp:641) of a driver without a timetable -
        # TTrainParameters("none"), Driver.cpp:1907
        "trainnumber": "none", "traincategory": "", "trainname": "", "train_brakingmassratio": 0.0,
        "train_enginetype": "", "train_engineload": 0.0, "train_stationfrom": "",
        "train_stationto": "", "train_stationindex": 0, "train_stationcount": 0,
        "train_stationstart": 0, "train_atpassengerstop": false, "train_length": 0.0,
        "scenario": "", "hours": 0, "minutes": 0, "seconds": 0, "air_temperature": 0.0,
        "light_level": 0.0,
        # update_screens() adds the touches of the screen (Train.cpp:10302)
        "touches": [],
    }
    for index:int in UNIVERSAL_COUNT:
        state["universal%d" % index] = false
    for field:String in EIM_TRAIN_FIELDS:
        state["eimp_t_" + field] = 0.0
    for car:int in range(1, EIM_CAR_COUNT + 1):
        for field:String in EIM_CAR_FIELDS:
            state["eimp_c%d_%s" % [car, field]] = 0.0
        for field:String in DIESEL_FIELDS:
            state["diesel_param_%d_%s" % [car, field]] = 0.0
        state["eimp_c%d_cv" % car] = 0.0
        for field:String in ["ms", "fuse", "batt", "conv", "heat"]:
            state["eimp_c%d_%s" % [car, field]] = false
        for field:String in ["pf", "pr", "comp_a", "comp_w"]:
            state["eimp_u%d_%s" % [car, field]] = false
    for car:int in range(1, CAR_COUNT + 1):
        for field:String in PRESSURE_FIELDS:
            state["eimp_pn%d_%s" % [car, field]] = 0.0
        state["brakes_%d_spring_active" % car] = false
        state["brakes_%d_spring_shutoff" % car] = false
        for field:String in ["doors_%d", "doors_l_%d", "doors_r_%d", "doorstep_l_%d", "doorstep_r_%d", "slip_%d"]:
            state[field % car] = false
        state["doors_no_%d" % car] = 0
        state["code_%d" % car] = ""
        state["car_name%d" % car] = ""
    _neutral_state = state
