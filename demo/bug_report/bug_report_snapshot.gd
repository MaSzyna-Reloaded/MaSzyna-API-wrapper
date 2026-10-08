class_name BugReportSnapshot
extends RefCounted

## The state of the game a problem report carries, read at the moment the report is opened: the
## hardware, the scenario, the simulation and its weather, the camera, the streaming, the vehicles
## and signal heads around the player and the events ahead (what already happened is in the
## recorder's gameplay log). Read once per report - the vehicles' dumps are no hot path here.

## Metres around the player's vehicle in which other vehicles and signal heads are reported
const NEARBY_RADIUS: float = 2000.0
## A signal head's light in its line, by SignallingServer.LightState: off, on, blinking
const LIGHT_LETTERS: Dictionary[SignallingServer.LightState, String] = {
    SignallingServer.LIGHT_STATE_OFF: ".",
    SignallingServer.LIGHT_STATE_ON: "O",
    SignallingServer.LIGHT_STATE_BLINKING: "*",
}


## Everything, by section; the world is null when no scenery is running
static func collect(world: SceneryWorld, started_msec: int) -> Dictionary:
    var origin: Vector3 = RailVehicleServer.vehicle_get_transform(PlayerServer.player_get_vehicle()).origin
    return {
        "hardware": hardware(),
        "scenario": scenario(world),
        "simulation": simulation(world, started_msec),
        "camera": camera(world),
        "streaming": streaming(),
        "vehicles": vehicles(origin),
        "signal_heads": signal_heads(origin),
        "events": {
            "upcoming": upcoming_events(),
        },
    }


static func hardware() -> Dictionary:
    var driver_info: PackedStringArray = OS.get_video_adapter_driver_info()
    return {
        "build": GameDataServer.build_get_number(),
        "godot": Engine.get_version_info().get("string", ""),
        "os": OS.get_name(),
        "os_version": OS.get_version(),
        "distribution": OS.get_distribution_name(),
        "architecture": Engine.get_architecture_name(),
        "debug": OS.is_debug_build(),
        "editor": OS.has_feature("editor"),
        "double_precision": OS.has_feature("double"),
        "cpu": OS.get_processor_name(),
        "cpu_threads": OS.get_processor_count(),
        "memory": OS.get_memory_info(),
        "gpu_vendor": RenderingServer.get_video_adapter_vendor(),
        "gpu": RenderingServer.get_video_adapter_name(),
        "gpu_type": enum_name(&"RenderingDevice", &"DeviceType", RenderingServer.get_video_adapter_type()),
        "gpu_driver": " ".join(driver_info),
        "graphics_api_version": RenderingServer.get_video_adapter_api_version(),
        "rendering_method": str(ProjectSettings.get_setting_with_override("rendering/renderer/rendering_method")),
        "rendering_driver": str(ProjectSettings.get_setting_with_override("rendering/rendering_device/driver")),
        "screen_size": vector_to_array(DisplayServer.screen_get_size()),
        "window_size": vector_to_array(DisplayServer.window_get_size()),
        "window_mode": enum_name(&"DisplayServer", &"WindowMode", DisplayServer.window_get_mode()),
        "locale": OS.get_locale(),
        "language": MaszynaTranslationServer.language,
    }


static func scenario(world: SceneryWorld) -> Dictionary:
    var vehicle: RID = PlayerServer.player_get_vehicle()
    var result: Dictionary = {
        "vehicle": VehicleServer.vehicle_get_name(vehicle),
        "driver_cabin": enum_name(&"RailVehicleCabinKind", &"Kind",
                RailVehicleServer.cabin_get_kind(RailVehicleServer.vehicle_get_driver_cabin(vehicle))),
    }
    if world:
        var scenery: MaszynaSceneryNode = world.get_scenery()
        result.merge({
            "filename": scenery.filename,
            "title": scenery.title,
            "start_time": scenery.start_time,
            "first_train_id": scenery.first_train_id,
        })
    return result


static func simulation(world: SceneryWorld, started_msec: int) -> Dictionary:
    var result: Dictionary = {
        "real_seconds_since_start": (Time.get_ticks_msec() - started_msec) / 1000.0,
        "simulation_time": SimulationServer.simulation_get_time(),
        "time_of_day": SimulationServer.time_of_day,
        "speed": SimulationServer.get_simulation_speed(),
        "current_speed": SimulationServer.simulation_get_current_speed(),
        "light_level": SimulationServer.get_light_level(),
    }
    if world:
        var environment: MaszynaEnvironmentNode = world.get_environment()
        result["date"] = "%04d-%02d-%02d" % [environment.year, environment.month, environment.day]
        result["weather"] = {
            "weather": MaszynaEnvironment.Weather.find_key(environment.weather),
            "season": MaszynaEnvironment.Season.find_key(environment.season),
            "cloudiness": environment.cloudiness,
            "precipitation": environment.precipitation,
            "temperature": environment.temperature,
            "air_temperature": SimulationServer.get_air_temperature(),
            "wind_direction": environment.wind_direction,
            "wind_strength": environment.wind_strength,
            "fog_enabled": environment.fog_enabled,
            "fog_density": environment.fog_density,
            "fog_distance": environment.fog_distance,
        }
    return result


static func camera(world: SceneryWorld) -> Dictionary:
    var result: Dictionary = {
        "mode": enum_name(&"PlayerCameraServer", &"CameraMode", PlayerCameraServer.camera_get_mode()),
        "target": VehicleServer.vehicle_get_name(PlayerCameraServer.camera_get_target()),
        "follow_view": enum_name(
                &"PlayerCameraServer", &"CameraFollowView", PlayerCameraServer.camera_get_follow_view()),
        "vehicle_transform": transform_to_dictionary(
                RailVehicleServer.vehicle_get_transform(PlayerServer.player_get_vehicle())),
    }
    var viewport_camera: Camera3D = world.get_viewport().get_camera_3d() if world else null
    if viewport_camera:
        result["transform"] = transform_to_dictionary(viewport_camera.global_transform)
    return result


static func streaming() -> Dictionary:
    return {
        "enabled": SceneryStreamingServer.streaming_is_enabled(),
        "building": SceneryStreamingServer.streaming_is_building(),
        "camera_position": vector_to_array(SceneryStreamingServer.streaming_get_camera_position()),
        "camera_chunk": vector_to_array(SceneryStreamingServer.streaming_get_camera_chunk()),
        "draw_distance": SceneryStreamingServer.streaming_get_draw_distance(),
        "statistics": SceneryStreamingServer.streaming_get_statistics(),
    }


## The player's trainset in full - state and configuration - and the state of every other vehicle
## within NEARBY_RADIUS of origin
static func vehicles(origin: Vector3) -> Dictionary:
    var trainset: Array[Dictionary] = []
    var nearby: Array[Dictionary] = []
    var vehicle: RID = PlayerServer.player_get_vehicle()
    var coupled: Array[RID] = []
    if vehicle.is_valid():
        coupled.assign(RailVehicleServer.vehicle_get_coupled(
                vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER))
    for rid: RID in coupled:
        trainset.append({
            "name": VehicleServer.vehicle_get_name(rid),
            "position": vector_to_array(RailVehicleServer.vehicle_get_transform(rid).origin),
            "state": VehicleServer.vehicle_dump_state(rid),
            "config": VehicleServer.vehicle_dump_config(rid),
        })
    for rid: RID in VehicleServer.vehicle_get_rids():
        var position: Vector3 = RailVehicleServer.vehicle_get_transform(rid).origin
        if rid in coupled or position.distance_to(origin) > NEARBY_RADIUS:
            continue
        nearby.append({
            "name": VehicleServer.vehicle_get_name(rid),
            "position": vector_to_array(position),
            "distance": position.distance_to(origin),
            "state": VehicleServer.vehicle_dump_state(rid),
        })
    return {"trainset": trainset, "nearby": nearby}


## Every signal head within NEARBY_RADIUS of origin, a line each, the nearest first:
## "<distance> <name> <aspect> <x>,<y>,<z> <lights>" - the lights a letter each in their order
## (LIGHT_LETTERS), "-" for an aspect never set
static func signal_heads(origin: Vector3) -> PackedStringArray:
    var nearby: Array[Array] = []
    for head: RID in SignallingServer.signal_head_get_rids():
        var position: Vector3 = E3DRenderingServer.instance_get_transform(
                SignallingServer.signal_head_get_instance(head)).origin
        var distance: float = position.distance_to(origin)
        if distance > NEARBY_RADIUS:
            continue
        var lights: String = ""
        for light: int in SignallingServer.signal_head_get_light_count(head):
            lights += LIGHT_LETTERS[SignallingServer.signal_head_get_light_state(head, light)]
        var aspect: String = SignallingServer.signal_head_get_aspect(head)
        nearby.append([distance, "%.1f %s %s %.1f,%.1f,%.1f %s" % [
            distance, SignallingServer.signal_head_get_name(head), aspect if aspect else "-",
            position.x, position.y, position.z, lights
        ]])
    nearby.sort_custom(func(a: Array, b: Array) -> bool: return a[0] < b[0])
    var result: PackedStringArray = []
    for head: Array in nearby:
        result.append(head[1])
    return result


## The queued events in the order they run, with the seconds left until each
static func upcoming_events() -> Array[Dictionary]:
    var result: Array[Dictionary] = []
    var now: float = SimulationServer.simulation_get_time()
    for event: RID in ScenarioEventServer.queue_get_events():
        result.append({
            "event": str(ScenarioEventServer.event_get_name(event)),
            "seconds_left": ScenarioEventServer.event_get_run_time(event) - now,
        })
    return result


## The name of an engine class's enum value (WINDOW_MODE_FULLSCREEN, not 3); the number when the
## enum has no such value
static func enum_name(class_name_of_enum: StringName, enum_of_class: StringName, value: int) -> String:
    for constant: String in ClassDB.class_get_enum_constants(class_name_of_enum, enum_of_class):
        if ClassDB.class_get_integer_constant(class_name_of_enum, constant) == value:
            return constant
    return str(value)


## A vector as a JSON array - JSON.stringify() would write its text
static func vector_to_array(vector: Variant) -> Array:
    if vector is Vector2 or vector is Vector2i:
        return [vector.x, vector.y]
    return [vector.x, vector.y, vector.z]


static func transform_to_dictionary(transform: Transform3D) -> Dictionary:
    return {
        "origin": vector_to_array(transform.origin),
        "rotation_radians": vector_to_array(transform.basis.get_euler()),
    }
