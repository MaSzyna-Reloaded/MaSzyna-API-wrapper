extends RefCounted
class_name LegacyCabinCabLights

## The lights of the cab itself, ported from the original cab layer: the cab light
## (TTrain::OnCommand_interiorlightenable/disable, Train.cpp:5244-5280), its dimmer
## (TTrain::OnCommand_interiorlightdimenable/disable, Train.cpp:6291-6340) and the instrument light
## (TTrain::InstrumentLightActive). Every cab keeps its own - a light left on stays in the cab it
## was switched on in - and the level it shines at follows the power of the vehicle
## (TTrain::Update(), Train.cpp:8436-8453).

const CAB_LIGHT:StringName = &"cablight_sw"
const CAB_LIGHT_DIM:StringName = &"cablightdim_sw"
const INSTRUMENT_LIGHT:StringName = &"instrumentlight_sw"
## A dimmed cab light's level (cablightlevel, Train.cpp:9745)
const DIMMED_LEVEL:float = 0.4
## The cab light's level fed without the 110 V converter (cablightlevel, Train.cpp:9745)
const LOW_VOLTAGE_LEVEL:float = 0.5

var _cab_light:Callable = _switch.bind(CAB_LIGHT)
var _cab_light_dim:Callable = _switch.bind(CAB_LIGHT_DIM)
var _instrument_light:Callable = _switch.bind(INSTRUMENT_LIGHT)
var _vehicle_rid:RID
var _cab:int


func control_ids() -> Array[StringName]:
    return [CAB_LIGHT, CAB_LIGHT_DIM, INSTRUMENT_LIGHT]


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, CAB_LIGHT, _cab_light)
    CabinSystem.register_control(vehicle_rid, cab, CAB_LIGHT_DIM, _cab_light_dim)
    CabinSystem.register_control(vehicle_rid, cab, INSTRUMENT_LIGHT, _instrument_light)
    CabinSystem.register_process(vehicle_rid, cab, _process)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, CAB_LIGHT, _cab_light)
    CabinSystem.unregister_control(_vehicle_rid, _cab, CAB_LIGHT_DIM, _cab_light_dim)
    CabinSystem.unregister_control(_vehicle_rid, _cab, INSTRUMENT_LIGHT, _instrument_light)
    CabinSystem.unregister_process(_vehicle_rid, _cab, _process)


func _switch(state:CabinState, action:StringName, value:Variant, control:StringName) -> Variant:
    state.set_value(control, state.is_pressed(control, action, value))
    _light(state)
    return null


func _process(state:CabinState, _delta:float) -> void:
    _light(state)


## The cab's lights as its switches and the vehicle's power leave them - lit only while 24 V or
## 110 V is there, the cab light at half without the 110 V converter (Train.cpp:8436-8453)
func _light(state:CabinState) -> void:
    var controller:RailVehicleController = VehicleServer.vehicle_get_controller(state.vehicle_rid) as RailVehicleController
    var power110:bool = controller and controller.get_power110_available()
    var powered:bool = power110 or (controller and controller.get_power24_available())
    var level:float = 0.0
    if powered and bool(state.get_value(CAB_LIGHT, false)):
        level = ((DIMMED_LEVEL if state.get_value(CAB_LIGHT_DIM, false) else 1.0)
                * (1.0 if power110 else LOW_VOLTAGE_LEVEL))
    CabinSystem.cab_set_light_level(_vehicle_rid, _cab, level)
    CabinSystem.cab_set_instrument_light_enabled(
            _vehicle_rid, _cab, powered and bool(state.get_value(INSTRUMENT_LIGHT, false)))
