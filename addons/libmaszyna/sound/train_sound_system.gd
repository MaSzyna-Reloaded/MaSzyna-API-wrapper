extends Node

const TRIGGER_INTERVAL:float = 0.05
const TRIGGER_MODE_TOGGLE:int = 0
const TRIGGER_MODE_CONTINUOUS:int = 1
const TRIGGER_MODE_CHANGE:int = 2
const EXTERIOR_CONTEXT:int = 5
const VOLUME_FACTOR:float = 2.0
const EXTERIOR_VOLUME_FACTOR:float = 1.0
const CABIN_UNIT_SIZE_FACTOR:float = 2.0
const EXTERIOR_UNIT_SIZE_FACTOR:float = 1.0
## How far into its own sample a vehicle's looping running noise may start (DynObj.cpp:6511).
const RUNNING_NOISE_MAX_START_FRACTION:float = 0.8
const CULLING_DISTANCE_SETTING:StringName = &"maszyna/sound/culling_distance"
## gnd_sfx/hard_cut_distance when the project does not set it (sfx_player_3d.gd:143)
const HARD_CUT_DISTANCE_DEFAULT:float = 1000.0
## The cab wall, as heard from inside: everything on the Exterior bus goes through one low-pass
## whose corner follows the listener's context. A barrier is a property of the barrier, not of
## each source, so it belongs on the bus - the per-source share of it is soundproofing, which
## already attenuates an external source by sqrt(0.2) for a closed cab.
const EXTERIOR_BUS:StringName = &"Exterior"
## The cab's own sounds (MmdSoundBankInstancer puts a cabin-only bank on it)
const CABIN_BUS:StringName = &"Cabin"
## The sound runs at the simulation's speed, lower and slower or higher and faster like a tape, up
## to this multiple, and stays at it above
const TAPE_MAX_SPEED:float = 4.0
## Set faster than this, the vehicles go silent (the weather window's speeds past the panel's x8)
const SILENT_ABOVE_SPEED:float = 8.0
## Corner frequency and trim per listener context: outside, closed cab, cab with an open window.
const WALL_OPEN_HZ:float = 20500.0
const WALL_CABIN_HZ:float = 1600.0
const WALL_WINDOW_HZ:float = 4000.0
const WALL_OPEN_DB:float = 0.0
const WALL_CABIN_DB:float = -2.0
const WALL_WINDOW_DB:float = -2.0
## Moving between the two is a transition, not a jump - a step in either clicks audibly.
const WALL_FADE_SECONDS:float = 0.15
## Cabin3D.get_sound_listener_context() returns this while the cab window is open.
const OPEN_WINDOW_CONTEXT:int = 3
## Update cadence for a bank right at the culling distance edge - banks closer to the listener
## interpolate down to 0.0 (every frame), matching prior behavior for nearby vehicles.
const FAR_UPDATE_INTERVAL:float = 0.5
## How often the distance to the listener is looked at. Between sweeps the frame only visits the
## banks that are in range.
const SWEEP_INTERVAL:float = 0.25

var DEFAULT_PROOFING:Array[PackedFloat32Array] = [
    PackedFloat32Array([1.0, sqrt(0.2), 1.0, sqrt(0.65), sqrt(0.2), sqrt(0.2)]),
    PackedFloat32Array([sqrt(0.2), 1.0, sqrt(0.2), sqrt(0.65), sqrt(0.65), sqrt(0.65)]),
    PackedFloat32Array([sqrt(0.2), sqrt(0.2), sqrt(0.2), sqrt(0.65), sqrt(0.65), 1.0]),
    PackedFloat32Array([sqrt(0.01), sqrt(0.01), sqrt(0.01), sqrt(0.2), sqrt(0.2), 1.0]),
    PackedFloat32Array([1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
]

class BankRuntime extends RefCounted:
    var player:SfxPlayer3D
    var vehicle:RailVehicle3D
    ## The vehicle's handle, valid once its simulation is - the key of its entry in _vehicle_events
    var vehicle_rid:RID = RID()
    var cabin_only:bool = false
    var enabled:bool = true
    var brake_sources:Dictionary = {}
    ## brake events this bank really has, resolved once they are built
    var brake_events:Array[BrakeEvent] = []
    var running:RunningSoundModel
    var soundproofing:Array[PackedFloat32Array] = []
    var triggers:Array[Trigger] = []
    var trigger_elapsed:float = 0.0
    var events_built:bool = false
    var anchored_cabin_instance_id:int = 0
    var sound_update_elapsed:float = 0.0
    ## Where this vehicle's looping running noise starts inside its own sample, as a fraction of
    ## it - drawn once, so every wagon of a trainset runs its copy out of phase with the others.
    var running_start_fraction:float = 0.0
    var culled:bool = false
    var last_batch:Dictionary = {}
    ## Within the culling distance, so the frame visits it - owned by _refresh_active_banks()
    var active:bool = false
    ## How often this bank is updated, from its distance to the listener
    var update_interval:float = 0.0


## One MMD sound source watched as a trigger. The descriptor a caller registers is a Dictionary
## of Variants; this is what it resolves to, once, because every field of it is read per tick per
## bank and a String()/StringName() conversion there is not free.
class Trigger extends RefCounted:
    var id:int = 0
    var state_property:String = ""
    var trigger_mode:int = 0
    var event_name:StringName = &""
    var parameter_name:StringName = &""
    var threshold_min:float = 0.0
    var threshold_max:float = 1.0
    var placement:StringName = &"general"
    var source:MmdSoundSourceDefinition
    ## Index into the vehicle's event counters, or -1 for a trigger reading vehicle state.
    ## Resolved once, in _add_trigger - the per-tick path never looks at the name.
    var event_index:int = -1
    ## Playing since the last time this trigger said so (TOGGLE/CONTINUOUS)
    var activated:bool = false
    ## The value this trigger last saw, for TRIGGER_MODE_CHANGE; INF until the first tick heard
    var last_value:float = INF
    ## The next start plays the opening bookend: cleared when the sound was due while out of
    ## earshot, set again only when the vehicle turns it off (sound.cpp:360-367, :498)
    var play_beginning:bool = true
    ## How far into its event a start without the opening bookend begins
    var beginning_length:float = 0.0


## One brake event of a bank, with the static tables of MaszynaBrakeSfxEventFactory already resolved
## into it - they are walked per event per tick otherwise.
class BrakeEvent extends RefCounted:
    var name:StringName = &""
    var parameter_names:Array[StringName] = []
    var state_keys:Array[String] = []
    var gate_key:String = ""
    var gate_on:float = 0.0
    var gate_off:float = 0.0
    var source:MmdSoundSourceDefinition


## The coupling elements the physics side reports, in the order of RailVehicleController.CouplingElement,
## attach first and detach second, then the pantograph events of RailVehicleElectricEngine - the layout
## of a vehicle's entry in _vehicle_events.
const VEHICLE_EVENT_INDICES:Dictionary[String, int] = {
    "coupler_sound/attach_coupler": 0,
    "coupler_sound/attach_brakehose": 1,
    "coupler_sound/attach_mainhose": 2,
    "coupler_sound/attach_control": 3,
    "coupler_sound/attach_gangway": 4,
    "coupler_sound/attach_heating": 5,
    "coupler_sound/detach_coupler": 6,
    "coupler_sound/detach_brakehose": 7,
    "coupler_sound/detach_mainhose": 8,
    "coupler_sound/detach_control": 9,
    "coupler_sound/detach_gangway": 10,
    "coupler_sound/detach_heating": 11,
    "pantograph_sound/up": 12,
    "pantograph_sound/down": 13,
}
const COUPLER_DETACH_OFFSET:int = 6
const PANTOGRAPH_UP_EVENT:int = 12
const PANTOGRAPH_DOWN_EVENT:int = 13
const VEHICLE_EVENT_COUNT:int = 14

var _banks:Dictionary = {}
## The bank of a vehicle, so looking one up does not mean scanning every bank in the scenery
var _banks_by_vehicle:Dictionary[RailVehicle3D, BankRuntime] = {}
## The banks within the culling distance - the only ones a frame visits
var _active:Array[BankRuntime] = []
## Coupling and pantograph one-shots are events, not vehicle state: the vehicle reports each
## attach, detach, pantograph up and down once, and the running counts the CHANGE triggers
## compare against live here, one entry per vehicle RID, shared by that vehicle's banks.
var _vehicle_events:Dictionary[RID, PackedInt32Array] = {}
## The electric engine each counted vehicle's pantograph events come from - a vehicle without one
## has no entry
var _pantograph_sources:Dictionary[RID, RailVehicleElectricEngine] = {}
## Vehicles of the listener's own trainset, refreshed with the sweep and on a context change
var _listener_trainset:Array[RID] = []
var _culling_distance:float = 1000.0
var _sweep_timer:Timer
var _listener:TrainSoundListener3D
var _next_trigger_id:int = 1
var _wall_tween:Tween


func _ready() -> void:
    set_process(false)
    _sweep_timer = Timer.new()
    _sweep_timer.wait_time = SWEEP_INTERVAL
    _sweep_timer.timeout.connect(_refresh_active_banks)
    add_child(_sweep_timer)
    _sweep_timer.start()
    SimulationServer.simulation_paused.connect(_on_runtime_paused)
    SimulationServer.simulation_unpaused.connect(_on_runtime_unpaused)
    SimulationServer.simulation_current_speed_changed.connect(_on_simulation_current_speed_changed)
    SimulationServer.simulation_speed_changed.connect(_mute_world)
    RailVehicleServer.vehicle_coupler_attached.connect(_on_coupler_attached)
    RailVehicleServer.vehicle_coupler_detached.connect(_on_coupler_detached)
    _on_simulation_current_speed_changed()


func set_listener(listener:TrainSoundListener3D) -> void:
    if _listener:
        _listener.context_changed.disconnect(_refresh_context)
    _listener = listener
    _listener.context_changed.connect(_refresh_context)
    _refresh_context()


func clear_listener(listener:TrainSoundListener3D) -> void:
    if not _listener == listener:
        return
    _listener.context_changed.disconnect(_refresh_context)
    _listener = null
    _refresh_context()


func register_bank(player:SfxPlayer3D, registration:Dictionary) -> void:
    var bank_id:int = player.get_instance_id()
    var runtime:BankRuntime = _banks.get(bank_id) as BankRuntime
    if not runtime:
        runtime = BankRuntime.new()
        runtime.player = player
        runtime.running_start_fraction = randf_range(0.0, RUNNING_NOISE_MAX_START_FRACTION)
        _banks[bank_id] = runtime
        player.tree_exiting.connect(_unregister_bank.bind(bank_id))
    _set_bank_vehicle(runtime, registration.get("vehicle") as RailVehicle3D)
    runtime.cabin_only = bool(registration.get("cabin_only", false))
    runtime.enabled = not runtime.cabin_only
    runtime.brake_sources = registration.get("brake_sources", {})
    runtime.running = registration.get("running") as RunningSoundModel
    runtime.soundproofing = registration.get("soundproofing", [])
    for descriptor:Dictionary in registration.get("triggers", []):
        _add_trigger(runtime, descriptor)
    _resolve_vehicle(runtime)
    _refresh_bank_context(runtime)
    _mark_active(runtime)


func register_trigger(player:SfxPlayer3D, descriptor:Dictionary) -> int:
    var bank_id:int = player.get_instance_id()
    var runtime:BankRuntime = _banks.get(bank_id) as BankRuntime
    if not runtime:
        runtime = BankRuntime.new()
        runtime.player = player
        _set_bank_vehicle(runtime, descriptor.get("vehicle") as RailVehicle3D)
        runtime.vehicle_rid = descriptor.get("vehicle_rid", RID())
        _banks[bank_id] = runtime
        player.tree_exiting.connect(_unregister_bank.bind(bank_id))
    var trigger_id:int = _add_trigger(runtime, descriptor)
    _mark_active(runtime)
    return trigger_id


func unregister_trigger(player:SfxPlayer3D, trigger_id:int) -> void:
    var runtime:BankRuntime = _banks.get(player.get_instance_id()) as BankRuntime
    if not runtime:
        return
    for index:int in range(runtime.triggers.size() - 1, -1, -1):
        if runtime.triggers[index].id == trigger_id:
            runtime.triggers.remove_at(index)
            break


## Only the banks the sweep left in range, and of those only the ones whose own interval is up.
## Everything that does not change with the frame - the distance, the culling, the listener's
## trainset - belongs to _refresh_active_banks().
func _process(delta:float) -> void:
    var states:Dictionary = {}
    for runtime:BankRuntime in _active:
        runtime.sound_update_elapsed += delta
        if runtime.sound_update_elapsed < runtime.update_interval:
            continue
        var elapsed:float = runtime.sound_update_elapsed
        runtime.sound_update_elapsed = fmod(
                runtime.sound_update_elapsed, maxf(runtime.update_interval, 0.001))

        if not states.has(runtime.vehicle_rid):
            states[runtime.vehicle_rid] = VehicleServer.vehicle_dump_state(runtime.vehicle_rid)
        var state:Dictionary = states[runtime.vehicle_rid]
        var batch:Dictionary = {}
        _update_brake_sounds(runtime, state, batch)
        _update_running_sounds(runtime, state, elapsed, batch)
        runtime.trigger_elapsed += delta
        if runtime.trigger_elapsed >= TRIGGER_INTERVAL:
            runtime.trigger_elapsed = fmod(runtime.trigger_elapsed, TRIGGER_INTERVAL)
            _update_triggers(runtime, state, batch)
        # Skip the modulate()/_apply_voice_state chain entirely when nothing actually changed
        # since last update - an idle in-range vehicle (engine off, no brake activity) would
        # otherwise still rebuild and re-apply the same voice state every update tick.
        if not batch == runtime.last_batch:
            runtime.player.set_parameters(batch)
            runtime.last_batch = batch


## Distance, culling and the set of banks a frame visits - the state this system owns about
## where the listener is. Driven by _sweep_timer, four times a second.
func _refresh_active_banks() -> void:
    # a player beyond its hard cut drops every play() silently, so the banks are culled - and
    # their triggers know they are unheard - no farther than that
    _culling_distance = minf(
            float(ProjectSettings.get_setting(CULLING_DISTANCE_SETTING, 1000.0)),
            float(ProjectSettings.get_setting(
                    SfxPlayer3D.HARD_CUT_DISTANCE_SETTING, HARD_CUT_DISTANCE_DEFAULT)))
    _refresh_listener_trainset()
    var listener_position:Vector3 = _listener.global_position if _listener else Vector3.ZERO
    for runtime:BankRuntime in _active:
        runtime.active = false
    _active.clear()
    for runtime:BankRuntime in _banks.values():
        if not runtime.vehicle_rid.is_valid() or not runtime.enabled:
            continue
        if not runtime.events_built and runtime.brake_sources:
            _build_brake_events(runtime)

        # Vehicles beyond every event's own max_distance are already inaudible - skip building
        # their sound state entirely instead of paying full per-frame cost (soundproofing,
        # play()/set_parameters()) for a scenery's worth of parked, unheard rolling stock.
        var distance:float = (
                runtime.vehicle.global_position.distance_to(listener_position)
                if runtime.vehicle and _listener else 0.0)
        if distance > _culling_distance:
            if not runtime.culled:
                runtime.culled = true
                _silence_bank(runtime)
            continue
        runtime.culled = false

        # Between the listener and the culling distance, update less often the farther away a
        # vehicle is - its sound is already quiet there, so a coarser update rate is inaudible.
        runtime.update_interval = lerpf(
                0.0, FAR_UPDATE_INTERVAL, clampf(distance / _culling_distance, 0.0, 1.0))
        _mark_active(runtime)

    if _active:
        set_process(true)
        return
    set_process(false)


## Takes a bank into the set the frame visits, before the next sweep has looked at it.
func _mark_active(runtime:BankRuntime) -> void:
    if runtime.active or not runtime.enabled or not runtime.vehicle_rid.is_valid():
        return
    runtime.active = true
    _active.append(runtime)
    set_process(true)


## The descriptor a caller registers is resolved into a Trigger here, once, and never read as a
## Dictionary again - see the class.
func _add_trigger(runtime:BankRuntime, descriptor:Dictionary) -> int:
    var trigger := Trigger.new()
    trigger.id = int(descriptor.get("id", 0))
    if trigger.id == 0:
        trigger.id = _next_trigger_id
        _next_trigger_id += 1
    trigger.state_property = String(descriptor.get("state_property", ""))
    trigger.event_index = VEHICLE_EVENT_INDICES.get(trigger.state_property, -1)
    trigger.trigger_mode = int(descriptor.get("trigger_mode", TRIGGER_MODE_TOGGLE))
    trigger.event_name = StringName(descriptor.get("sound_event", &""))
    trigger.parameter_name = StringName(descriptor.get("sound_parameter", &""))
    trigger.threshold_min = float(descriptor.get("trigger_threshold_min", 0.0))
    trigger.threshold_max = float(descriptor.get("trigger_threshold_max", 1.0))
    trigger.placement = StringName(descriptor.get("sound_placement", &"general"))
    trigger.source = descriptor.get("source") as MmdSoundSourceDefinition
    if trigger.source:
        trigger.beginning_length = MmdSoundEventBuilder.stream_length(trigger.source.sound_begin)
    runtime.triggers.append(trigger)
    return trigger.id


## Resolves the bank's vehicle and, for the first bank of a vehicle, starts counting that
## vehicle's coupling and pantograph events. The counts belong here rather than in the vehicle state: a coupling
## is an event the physics side reports once, and what a CHANGE trigger needs is a number that
## only ever goes up.
func _resolve_vehicle(runtime:BankRuntime) -> void:
    if not runtime.vehicle:
        return
    var vehicle_rid:RID = runtime.vehicle.get_rid()
    if not VehicleServer.vehicle_is_simulation_ready(vehicle_rid):
        vehicle_rid = RID()
    if not runtime.vehicle_rid == vehicle_rid:
        # the vehicle took its controller's handle in place of its own
        # (RailVehicle3D::set_controller) - the counting moves to the new one
        _stop_counting_events(runtime.vehicle_rid)
        _vehicle_events.erase(runtime.vehicle_rid)
    runtime.vehicle_rid = vehicle_rid
    if not vehicle_rid.is_valid():
        return
    # the vehicle was built again: its engine is another component now
    _stop_counting_events(vehicle_rid)
    if not _vehicle_events.has(vehicle_rid):
        var counts:PackedInt32Array = PackedInt32Array()
        counts.resize(VEHICLE_EVENT_COUNT)
        _vehicle_events[vehicle_rid] = counts
    var engine:RailVehicleElectricEngine = VehicleServer.vehicle_component_get(
            vehicle_rid, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleElectricEngine
    if engine:
        _pantograph_sources[vehicle_rid] = engine
        engine.pantograph_up.connect(_on_pantograph_up.bind(vehicle_rid))
        engine.pantograph_down.connect(_on_pantograph_down.bind(vehicle_rid))


## Disconnects a vehicle's pantograph events; its counts stay, the caller decides about them.
func _stop_counting_events(vehicle_rid:RID) -> void:
    var engine:RailVehicleElectricEngine = _pantograph_sources.get(vehicle_rid)
    if is_instance_valid(engine):
        engine.pantograph_up.disconnect(_on_pantograph_up.bind(vehicle_rid))
        engine.pantograph_down.disconnect(_on_pantograph_down.bind(vehicle_rid))
    _pantograph_sources.erase(vehicle_rid)


## Every rail vehicle reports its couplings; only the counted ones are kept
func _on_coupler_attached(vehicle_rid:RID, element:RailVehicleController.CouplingElement) -> void:
    if _vehicle_events.has(vehicle_rid):
        _vehicle_events[vehicle_rid][element] += 1


func _on_coupler_detached(vehicle_rid:RID, element:RailVehicleController.CouplingElement) -> void:
    if _vehicle_events.has(vehicle_rid):
        _vehicle_events[vehicle_rid][COUPLER_DETACH_OFFSET + element] += 1


## Both pantographs count as one event: the vehicle has one sound for them (sPantUp,
## DynObj.cpp:3881); where on the roof it plays is in TODO.md.
func _on_pantograph_up(_selector:int, vehicle_rid:RID) -> void:
    _vehicle_events[vehicle_rid][PANTOGRAPH_UP_EVENT] += 1


func _on_pantograph_down(_selector:int, vehicle_rid:RID) -> void:
    _vehicle_events[vehicle_rid][PANTOGRAPH_DOWN_EVENT] += 1


## Builds the bank's brake events from its MMD sources and resolves the static tables of
## MaszynaBrakeSfxEventFactory into BrakeEvents. Runs once, as soon as the sweep sees a controller -
## the config it needs does not exist before that.
func _build_brake_events(runtime:BankRuntime) -> void:
    var built:Array[SfxEvent] = MaszynaBrakeSfxEventFactory.build_events(
            runtime.brake_sources, VehicleServer.vehicle_dump_config(runtime.vehicle_rid))
    if not built:
        return
    var events:Array[SfxEvent] = runtime.player.bank.events.duplicate()
    events.append_array(built)
    runtime.player.bank.events = events
    runtime.events_built = true
    for event_name:StringName in MaszynaBrakeSfxEventFactory.EVENT_PARAMETERS:
        if not runtime.player.bank.get_event(event_name):
            continue
        var brake_event := BrakeEvent.new()
        brake_event.name = event_name
        var parameters:Dictionary = MaszynaBrakeSfxEventFactory.EVENT_PARAMETERS[event_name]
        for parameter_name:StringName in parameters:
            brake_event.parameter_names.append(parameter_name)
            brake_event.state_keys.append(String(parameters[parameter_name]))
        var gate:Array = MaszynaBrakeSfxEventFactory.EVENT_GATES.get(event_name, [])
        if gate:
            brake_event.gate_key = String(gate[0])
            brake_event.gate_on = float(gate[1])
            brake_event.gate_off = float(gate[2])
        brake_event.source = _primary_source(runtime, event_name)
        runtime.brake_events.append(brake_event)
    runtime.anchored_cabin_instance_id = 0
    _update_spatial_anchors(runtime)


func _update_brake_sounds(runtime:BankRuntime, state:Dictionary, batch:Dictionary) -> void:
    for brake_event:BrakeEvent in runtime.brake_events:
        var event_parameters:Dictionary = {}
        var has_active_parameter:bool = false
        for index:int in range(brake_event.parameter_names.size()):
            var value:float = _parameter_value(state.get(brake_event.state_keys[index], 0.0))
            event_parameters[brake_event.parameter_names[index]] = value
            has_active_parameter = has_active_parameter or not is_zero_approx(value)
        var playing:bool = runtime.player.is_playing(brake_event.name)
        if brake_event.gate_key:
            has_active_parameter = _parameter_value(state.get(brake_event.gate_key, 0.0)) > (
                    brake_event.gate_off if playing else brake_event.gate_on)
            if playing and not has_active_parameter:
                runtime.player.stop(brake_event.name, false)
                continue
        # an idle, silent brake event needs no listener-dependent parameters at all
        if not playing and not has_active_parameter:
            continue
        event_parameters[&"soundproofing"] = _soundproofing(runtime, brake_event.source)
        event_parameters[&"unit_size"] = _unit_size_factor(runtime)
        event_parameters[&"gain"] = _volume_factor(runtime)
        if not playing:
            runtime.player.play(brake_event.name, event_parameters)
        batch[brake_event.name] = event_parameters


func _update_triggers(runtime:BankRuntime, state:Dictionary, batch:Dictionary) -> void:
    var vehicle_events:PackedInt32Array = _vehicle_events.get(runtime.vehicle_rid, PackedInt32Array())
    for trigger:Trigger in runtime.triggers:
        var value:float = 0.0
        if trigger.event_index < 0:
            value = _parameter_value(state.get(trigger.state_property, 0.0))
        elif vehicle_events:
            value = float(vehicle_events[trigger.event_index])
        if trigger.trigger_mode == TRIGGER_MODE_CHANGE:
            var previous_value:float = trigger.last_value
            trigger.last_value = value
            if is_inf(previous_value) or is_equal_approx(previous_value, value):
                continue
            runtime.player.play(trigger.event_name, _trigger_parameters(runtime, trigger, state, value))
            continue

        var should_play:bool = value <= trigger.threshold_max and value >= trigger.threshold_min
        if trigger.trigger_mode == TRIGGER_MODE_TOGGLE:
            should_play = not is_zero_approx(value) and should_play
        if not should_play:
            trigger.play_beginning = true
        # a silent trigger that stays silent needs none of the listener-dependent parameters
        if not should_play and not trigger.activated:
            continue

        var parameters:Dictionary = _trigger_parameters(runtime, trigger, state, value)
        if should_play and not trigger.activated:
            runtime.player.play(
                    trigger.event_name,
                    0.0 if trigger.play_beginning else trigger.beginning_length,
                    parameters)
            trigger.play_beginning = false
            trigger.activated = true
        elif not should_play and trigger.activated:
            runtime.player.stop(trigger.event_name, false)
            trigger.activated = false
        if should_play and parameters and runtime.player.is_playing(trigger.event_name):
            batch[trigger.event_name] = parameters


func _trigger_parameters(
        runtime:BankRuntime, trigger:Trigger, state:Dictionary, value:float) -> Dictionary:
    var parameters:Dictionary = {}
    if trigger.trigger_mode == TRIGGER_MODE_CONTINUOUS and trigger.parameter_name:
        parameters[trigger.parameter_name] = value
    if trigger.source:
        if trigger.source.label == "engine":
            parameters[&"engine_gain"] = _engine_gain(runtime, state, trigger.source)
        parameters[&"soundproofing"] = _soundproofing(runtime, trigger.source)
        return parameters
    if not trigger.placement == &"general":
        parameters[&"soundproofing"] = _placement_soundproofing(runtime, trigger.placement)
    return parameters


func _update_running_sounds(
        runtime:BankRuntime, state:Dictionary, elapsed:float, batch:Dictionary) -> void:
    if not runtime.running:
        return
    var results:Dictionary = runtime.running.update(
            runtime.vehicle_rid, state, elapsed, not _listener_trainset.has(runtime.vehicle_rid))
    for event_name:StringName in results:
        var result:Dictionary = results[event_name]
        var action:int = result["action"]
        if action == RunningSoundModel.Action.STOP:
            if runtime.player.is_playing(event_name):
                runtime.player.stop(event_name, false)
            continue
        var parameters:Dictionary = result["parameters"]
        parameters[&"soundproofing"] = _soundproofing(runtime, result["source"])
        if action == RunningSoundModel.Action.ONE_SHOT:
            runtime.player.play(event_name, parameters)
            continue
        if not runtime.player.is_playing(event_name):
            # every vehicle of a trainset plays the same recording, and started together they
            # comb-filter into a metallic ring heard from outside. Each one starts its own copy
            # further into the sample (DynObj.cpp:6511, audiorenderer.cpp:99).
            runtime.player.play(event_name, null, parameters, runtime.running_start_fraction)
        batch[event_name] = parameters


## The trainset driven from the listener's cab - its outer noise is replaced by the cab running
## noise (DynObj.cpp:4632-4640). It changes when the listener changes cab or the trainset is
## recoupled, so it is walked with the sweep and not per frame.
func _refresh_listener_trainset() -> void:
    _listener_trainset.clear()
    if not _listener or not _listener.listener_cabin or not _listener.listener_vehicle:
        return
    _listener_trainset.assign(RailVehicleServer.vehicle_get_coupled(
            _listener.listener_vehicle.get_rid(), 0, RailVehicleController.COUPLING_ELEMENT_COUPLER))


func _engine_gain(
        runtime:BankRuntime, state:Dictionary, source:MmdSoundSourceDefinition) -> float:
    var rpm_ratio:float = clampf(float(state.get("engine_rpm_ratio", 0.0)), 0.0, 1.0)
    var nominal_power:float = float(VehicleServer.vehicle_dump_config(runtime.vehicle_rid).get("power", 0.0))
    var load_ratio:float = 0.0
    if nominal_power > 0.0:
        load_ratio = maxf(float(state.get("engine_power", 0.0)) / nominal_power, 0.0)
    var level:float = 0.25 * load_ratio + 0.75 * rpm_ratio
    return clampf(source.amplitude_offset + source.amplitude_factor * level, 0.0, 2.0)


func _refresh_context() -> void:
    _refresh_exterior_wall()
    for runtime:BankRuntime in _banks.values():
        _refresh_bank_context(runtime)
    _refresh_active_banks()


## Follows the listener into and out of the cab: outside is open, a closed cab is muffled, an
## open window is most of the way back to open.
func _refresh_exterior_wall() -> void:
    var filter:AudioEffectLowPassFilter = _exterior_wall_filter()
    if not filter:
        return
    var cutoff_hz:float = WALL_OPEN_HZ
    var volume_db:float = WALL_OPEN_DB
    if _listener and _listener.listener_cabin:
        var open_window:bool = _listener.listener_context == OPEN_WINDOW_CONTEXT
        cutoff_hz = WALL_WINDOW_HZ if open_window else WALL_CABIN_HZ
        volume_db = WALL_WINDOW_DB if open_window else WALL_CABIN_DB
    if _wall_tween:
        _wall_tween.kill()
    _wall_tween = create_tween().set_parallel()
    _wall_tween.tween_property(filter, "cutoff_hz", cutoff_hz, WALL_FADE_SECONDS)
    _wall_tween.tween_method(
        _set_exterior_bus_volume,
        AudioServer.get_bus_volume_db(AudioServer.get_bus_index(EXTERIOR_BUS)),
        volume_db,
        WALL_FADE_SECONDS
    )


func _set_exterior_bus_volume(volume_db:float) -> void:
    AudioServer.set_bus_volume_db(AudioServer.get_bus_index(EXTERIOR_BUS), volume_db)


## The wall is the first effect of the Exterior bus, but it is looked up by type rather than by
## index so reordering the chain in the bus layout cannot silently retune something else.
func _exterior_wall_filter() -> AudioEffectLowPassFilter:
    var bus_index:int = AudioServer.get_bus_index(EXTERIOR_BUS)
    if bus_index < 0:
        return null
    for effect_index:int in range(AudioServer.get_bus_effect_count(bus_index)):
        var effect:AudioEffect = AudioServer.get_bus_effect(bus_index, effect_index)
        if effect is AudioEffectLowPassFilter:
            return effect
    return null


func _refresh_bank_context(runtime:BankRuntime) -> void:
    var enabled:bool = not runtime.cabin_only or _inside_vehicle(runtime.vehicle)
    if not runtime.enabled == enabled:
        runtime.enabled = enabled
        if not enabled:
            _silence_bank(runtime)
    if enabled:
        _update_spatial_anchors(runtime)


## The bank goes out of earshot: its sounds stop, each one still running resumes past its opening
## bookend when heard again, and a one-shot due while unheard is dropped (sound.cpp:360-367)
func _silence_bank(runtime:BankRuntime) -> void:
    runtime.player.stop(false)
    for trigger:Trigger in runtime.triggers:
        trigger.activated = false
        trigger.play_beginning = false
        trigger.last_value = INF
    runtime.last_batch = {}


func _inside_vehicle(vehicle:RailVehicle3D) -> bool:
    return not _listener == null and not _listener.listener_cabin == null \
            and _listener.listener_vehicle == vehicle


func _soundproofing(runtime:BankRuntime, source:MmdSoundSourceDefinition) -> float:
    if not source:
        return 1.0
    if source.placement == &"general":
        return 1.0
    return _placement_soundproofing(runtime, source.placement, source.soundproofing)


func _placement_soundproofing(
        runtime:BankRuntime, placement_name:StringName,
        source_profile:PackedFloat32Array = PackedFloat32Array()) -> float:
    var placement:int = _placement_index(placement_name)
    var inside_source:bool = _inside_vehicle(runtime.vehicle)
    if placement == 0 and inside_source:
        return _source_profile_value(
                source_profile, runtime.soundproofing, placement, _listener.listener_context)
    if not _listener or not _listener.listener_vehicle:
        return 0.0 if placement == 0 else _source_profile_value(
                source_profile, runtime.soundproofing, placement, EXTERIOR_CONTEXT)
    var source_context:int = _listener.listener_context if inside_source else EXTERIOR_CONTEXT
    var proofing:float = _source_profile_value(
            source_profile, runtime.soundproofing, placement, source_context)
    if not _listener.listener_vehicle == runtime.vehicle:
        proofing *= _profile_value(
                _vehicle_profile(_listener.listener_vehicle), 2, _listener.listener_context)
    return proofing


func _source_profile_value(
        source_profile:PackedFloat32Array, profile:Array[PackedFloat32Array],
        placement:int, context:int) -> float:
    if source_profile.size() == 6 and not is_equal_approx(source_profile[context], -1.0):
        return sqrt(clampf(source_profile[context], 0.0, 1.0))
    return _profile_value(profile, placement, context)


func _profile_value(profile:Array[PackedFloat32Array], placement:int, context:int) -> float:
    if profile.size() == 5:
        var value:float = profile[placement][context]
        if not is_equal_approx(value, -1.0):
            return sqrt(clampf(value, 0.0, 1.0))
    return DEFAULT_PROOFING[placement][context]


## Looked up rather than searched: a scenery has hundreds of banks and this used to scan all of
## them on every call, several times per frame
func _vehicle_profile(vehicle:RailVehicle3D) -> Array[PackedFloat32Array]:
    var runtime:BankRuntime = _banks_by_vehicle.get(vehicle)
    # a typed empty one: a bare [] is refused by the typed parameter it is handed to
    var none:Array[PackedFloat32Array] = []
    return runtime.soundproofing if runtime else none


func _placement_index(placement:StringName) -> int:
    match placement:
        &"internal": return 0
        &"engine": return 1
        &"external": return 2
        &"custom": return 4
    return 3


func _unit_size_factor(runtime:BankRuntime) -> float:
    if _inside_vehicle(runtime.vehicle):
        return CABIN_UNIT_SIZE_FACTOR
    return EXTERIOR_UNIT_SIZE_FACTOR


func _volume_factor(runtime:BankRuntime) -> float:
    var factor:float = VOLUME_FACTOR
    if not _inside_vehicle(runtime.vehicle):
        factor *= EXTERIOR_VOLUME_FACTOR
    return factor


func _primary_source(runtime:BankRuntime, event_name:StringName) -> MmdSoundSourceDefinition:
    for label:String in MaszynaBrakeSfxEventFactory.EVENT_LABEL_GROUPS.get(event_name, []):
        if runtime.brake_sources.has(label):
            return runtime.brake_sources[label] as MmdSoundSourceDefinition
    for trigger:Trigger in runtime.triggers:
        if trigger.event_name == event_name:
            return trigger.source
    return null


func _update_spatial_anchors(runtime:BankRuntime) -> void:
    var cabin:Cabin3D = _listener.listener_cabin if _listener else null
    # Home/End rebuilds the controls inside the same cabin node, so the occupied cab is part of the key.
    var anchor_key:int = hash([cabin.get_instance_id(), cabin.cab_number]) if cabin else 0
    if not cabin or not cabin.get_parent() == runtime.vehicle \
            or anchor_key == runtime.anchored_cabin_instance_id:
        return
    runtime.anchored_cabin_instance_id = anchor_key
    var brake_anchor:Vector3 = _cabin_anchor(runtime.vehicle, cabin, "brakectrl_")
    var local_anchor:Vector3 = _cabin_anchor(runtime.vehicle, cabin, "localbrake_")
    if local_anchor == Vector3.ZERO:
        local_anchor = brake_anchor
    var fallback:Vector3 = (cabin.camera_bound_min + cabin.camera_bound_max) * 0.5 + Vector3.UP
    if brake_anchor == Vector3.ZERO:
        brake_anchor = fallback
    if local_anchor == Vector3.ZERO:
        local_anchor = fallback
    _apply_anchor(runtime, &"pipe_hiss", brake_anchor)
    _apply_anchor(runtime, &"local_brake_hiss", local_anchor)
    # Train.cpp:10281 - the Hasler ticks from its own needle (dsbHasler->offset(gauge.model_offset())).
    var tacho_anchor:Vector3 = _cabin_anchor(runtime.vehicle, cabin, "tachometer_")
    if not tacho_anchor == Vector3.ZERO:
        _apply_anchor(runtime, &"tachoclock", tacho_anchor)


func _apply_anchor(runtime:BankRuntime, event_name:StringName, position:Vector3) -> void:
    var source:MmdSoundSourceDefinition = _primary_source(runtime, event_name)
    if not source or not source.offset == Vector3.ZERO:
        return
    var event:SfxEvent = runtime.player.bank.get_event(event_name)
    if event and event.spatial_config:
        event.spatial_config.position = position


func _cabin_anchor(vehicle:RailVehicle3D, cabin:Cabin3D, prefix:String) -> Vector3:
    var widget:Node = cabin.find_child("%s*" % prefix, true, false)
    if not widget:
        return Vector3.ZERO
    var mesh_path:Variant = widget.get("mesh_path")
    if mesh_path == null:
        mesh_path = widget.get("target_mesh_path")
    var mesh:Node3D = widget.get_node_or_null(mesh_path) as Node3D if mesh_path else null
    return vehicle.to_local(mesh.global_position) if mesh else Vector3.ZERO


func _parameter_value(raw:Variant) -> float:
    if typeof(raw) == TYPE_BOOL:
        return 1.0 if raw else 0.0
    return float(raw) if raw else 0.0


## The one writer of a bank's vehicle: the announcement the bank reacts to is wired with it, so
## the two can never disagree about which vehicle the bank is listening to.
func _set_bank_vehicle(runtime:BankRuntime, vehicle:RailVehicle3D) -> void:
    if runtime.vehicle == vehicle:
        return
    var previous:RailVehicle3D = runtime.vehicle
    runtime.vehicle = vehicle
    if previous and not _has_bank_of_vehicle_node(previous):
        # a bank is unregistered from its player's tree_exiting, which is the vehicle being freed:
        # by then the vehicle may already be gone, and it takes its connections with it
        if is_instance_valid(previous):
            previous.controller_changed.disconnect(_on_vehicle_controller_changed.bind(previous))
        _banks_by_vehicle.erase(previous)
    if not vehicle:
        return
    # One connection per vehicle rather than per bank: a vehicle carries several banks (exterior,
    # cabin), connections live on the emitter, and two banks binding the same method to the same
    # vehicle count as one - the second would never hear the announcement.
    if not _banks_by_vehicle.has(vehicle):
        # a bank is registered while its vehicle is still being built, so its controller comes
        # from the vehicle's own announcement rather than being looked for again later
        vehicle.controller_changed.connect(_on_vehicle_controller_changed.bind(vehicle))
    _banks_by_vehicle[vehicle] = runtime


## The vehicle has a different controller now - or its first one. Every bank it carries takes it.
func _on_vehicle_controller_changed(vehicle:RailVehicle3D) -> void:
    for runtime:BankRuntime in _banks.values():
        if runtime.vehicle == vehicle:
            _resolve_vehicle(runtime)
            _refresh_bank_context(runtime)


## Whether any bank still points at this vehicle - the connection above lives as long as one does.
func _has_bank_of_vehicle_node(vehicle:RailVehicle3D) -> bool:
    for runtime:BankRuntime in _banks.values():
        if runtime.vehicle == vehicle:
            return true
    return false


## The world is paused (SimulationServer.simulation_pause()): the system stops updating the banks
func _on_runtime_paused() -> void:
    process_mode = Node.PROCESS_MODE_DISABLED
    _mute_world()


## The whole audio's speed - the UI's sounds too (TODO.md)
func _on_simulation_current_speed_changed() -> void:
    AudioServer.playback_speed_scale = minf(SimulationServer.simulation_get_current_speed(), TAPE_MAX_SPEED)


func _on_runtime_unpaused() -> void:
    process_mode = Node.PROCESS_MODE_INHERIT
    _mute_world()


## The buses the vehicles are heard on are silent while the world is paused or set to run faster than
## SILENT_ABOVE_SPEED - with every voice on them, including those that start meanwhile (a scenery
## being loaded). The speed set, not the running one: coming down from above, the running speed
## approaches the limit from above for seconds.
func _mute_world() -> void:
    var silent:bool = SimulationServer.simulation_is_paused() or SimulationServer.simulation_speed > SILENT_ABOVE_SPEED
    AudioServer.set_bus_mute(AudioServer.get_bus_index(CABIN_BUS), silent)
    AudioServer.set_bus_mute(AudioServer.get_bus_index(EXTERIOR_BUS), silent)


func _unregister_bank(bank_id:int) -> void:
    var removed:BankRuntime = _banks.get(bank_id)
    if not removed:
        return
    _set_bank_vehicle(removed, null)
    _banks.erase(bank_id)
    if removed.vehicle_rid.is_valid() and not _has_bank_of_vehicle(removed.vehicle_rid):
        _stop_counting_events(removed.vehicle_rid)
        _vehicle_events.erase(removed.vehicle_rid)
    if removed.active:
        removed.active = false
        _active.erase(removed)


## A vehicle's coupling-event counting is shared by its banks, so it outlives any one of them.
func _has_bank_of_vehicle(vehicle_rid:RID) -> bool:
    for runtime:BankRuntime in _banks.values():
        if runtime.vehicle_rid == vehicle_rid:
            return true
    return false
