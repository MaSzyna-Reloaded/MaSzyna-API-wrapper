extends Cabin3D
class_name MaszynaDynamicTrainCabin

## MMD-driven cabin builder, analogous to E3DModelInstance/MaszynaRailVehiclePhysicsNode: given
## data_path/mmd_filename/skin it parses the vehicle's MMD file, resolves cab0/cab1/cab2 from the
## controller's cabin_occupied state, and builds a real, interactive cabin (Etap A+B scope -
## see mmd_cabin_instancer.gd) instead of requiring a hand-authored cabin_scene.
##
## Deliberately overrides _ready() and does not call super(): the base Cabin3D._ready() emits
## cabin_ready immediately, before this class's own children (cab model, widgets) exist -
## readiness here must wait until the whole MMD-derived "Generated" subtree is actually built.
## Everything below runs synchronously within one _ready() call (MMD parsing and E3D loading
## are both synchronous), so CabinSystem.vehicle_show_cabin()'s wait on cabin_ready still resolves
## within the same add_child() call that creates this node.
##
## A view of the vehicle's cab only: the cab logic it shows is the vehicle's
## (MaszynaLegacyVehicleSystem attaches it while the vehicle is driven), and the player's keys reach it
## without this node (MaszynaPlayer).

@export var data_path:String = ""
@export var mmd_filename:String = ""
@export var skin:String = ""

## Cab light height above the driver's eyes, kept under the top of the cab model - the model top
## is its outer shell (roof pipes, heaters), not the ceiling the light has to stay under.
const CAB_LIGHT_ABOVE_DRIVER:float = 0.4
const CAB_LIGHT_CEILING_OFFSET:float = 0.15
## Quirk: MMD has no cab light position (cablight: holds only unused colors, Train.cpp:118-133), so the
## light goes to the cab model's ceiling lamp, found by the submodel names the game's cab models use
## for it (most specific first); CAB_LIGHT_ABOVE_DRIVER is the fallback without such a lamp.
const CAB_LAMP_SUBMODEL_NAMES:Array[String] = [
    "lampa_suf0", "lampy_sufit", "lampa_sufi", "lampa_sufit", "lampasufitowa", "lampasufit", "swiatlo_sufit",
    "cablight", "lampa",
]
## The radio message played on the cab's radio (_on_radio_message_sent())
const RADIO_MESSAGE:StringName = &"radio_message"
## The quietest gain a radio turned all the way down plays at, above linear_to_db()'s -inf
const MUTED_GAIN:float = 0.0001
## Distance of the cab light below the found ceiling lamp - inside the lamp's shadow casting mesh
## it would light nothing.
const CAB_LIGHT_BELOW_LAMP:float = 0.05
## No rain falls inside the vehicle the player sits in: its body, as FIZ Dimensions give it, takes
## the precipitation away. Only the shown cab has one - a volume on every vehicle of a scenery was
## a quarter of the frame (docs/findings-archive.md, 2026-10-03 hundreds of vehicles)
const RAIN_EXCLUSION_NAME:StringName = &"RainExclusion"
const RAIN_EXCLUSION_PRECIPITATION_DELTA:float = -1.0

var _generated:Node3D
var _diagnostics:Array[Dictionary] = []
var _random_choices:Dictionary = {}
var _last_cab_number:int = 0
## The cab model's meshes as CabinHUDMouseSystem occluders - the desk hides what runs under it
var _occluders:Array[RID] = []
## The cab radio's loudspeaker
var _radio_player:SfxPlayer


func _ready() -> void:
    vehicle_rid_changed.connect(_on_vehicle_rid_changed)
    _radio_player = SfxPlayer.new()
    _radio_player.name = "RadioSfxPlayer"
    add_child(_radio_player)
    CabinSystem.radio_message_sent.connect(_on_radio_message_sent)
    # the MMD and the models are the game directory's
    GameDataServer.data_reload_requested.connect(reload)
    ProjectSettings.settings_changed.connect(_apply_reverse_cull_face)
    # controller_path (inherited from Cabin3D) may already name the vehicle when this cab is
    # placed in a scene rather than built by CabinSystem.vehicle_show_cabin(), which names it itself.
    if controller_path:
        var physics_node:VehiclePhysicsNode = get_node_or_null(controller_path)
        set_vehicle_rid(physics_node.vehicle_rid if physics_node else "")
    # Cabin3D's own _ready() emits cabin_ready; the engine calls it beside this one.


## Cabin3D announces the vehicle rather than letting a subclass override set_vehicle_rid(): the
## vehicle calls that method typed, so a script method of the same name would never run.
func _on_vehicle_rid_changed(_vehicle_rid:RID) -> void:
    if not CabinSystem.vehicle_cabin_occupied_changed.is_connected(_on_cabin_occupied_changed):
        CabinSystem.vehicle_cabin_occupied_changed.connect(_on_cabin_occupied_changed)
    _rebuild_generated()


func _exit_tree() -> void:
    # the announcement goes first: clearing the vehicle would otherwise rebuild the cab on its
    # way out of the tree
    vehicle_rid_changed.disconnect(_on_vehicle_rid_changed)
    CabinSystem.radio_message_sent.disconnect(_on_radio_message_sent)
    GameDataServer.data_reload_requested.disconnect(reload)
    ProjectSettings.settings_changed.disconnect(_apply_reverse_cull_face)
    CabinSystem.vehicle_cabin_occupied_changed.disconnect(_on_cabin_occupied_changed)
    set_vehicle_rid(RID())
    _free_occluders()


## TTrain::radio_message() (Train.cpp:11034-11049): a message within reach of the vehicle, heard
## on its radio switched on, powered and tuned to the message's channel, at the radio's volume.
## The original plays the others muted and raises them when the radio is tuned mid-message
## (update_sounds_radio(), Train.cpp:10251-10268); here they are not played (TODO.md). Its
## transcript is shown when it is heard at all (sound_source::update_counter(), sound.cpp:955).
func _on_radio_message_sent(
    message:SfxEvent, transcript:Transcript, channel:int, position:Vector3, reach:float
) -> void:
    var vehicle:RID = get_vehicle_rid()
    if not vehicle or message == null:
        return
    if reach > 0.0 and RailVehicleServer.vehicle_get_transform(vehicle).origin.distance_to(position) > reach:
        return
    var state:Dictionary = CabinSystem.vehicle_state(vehicle)
    if not (state.get("radio_enabled", false) and state.get("radio_powered", false)
            and int(state.get("radio_channel", 0)) == channel):
        return
    var played:SfxEvent = message.duplicate(true)
    played.name = RADIO_MESSAGE
    played.spatial_config = null
    played.master_track.volume_db += linear_to_db(maxf(float(state.get("radio_volume", 0.0)), MUTED_GAIN))
    var bank:SfxBank = SfxBank.new()
    var events:Array[SfxEvent] = [played]
    bank.events = events
    _radio_player.bank = bank
    _radio_player.play(RADIO_MESSAGE)
    if transcript and float(state.get("radio_volume", 0.0)) > 0.0:
        TranscriptSystem.add(transcript)


func get_diagnostics() -> Array[Dictionary]:
    return _diagnostics


func reload() -> void:
    _rebuild_generated()


## Rebuilds when the crew moves to another cab (cab0 = machine room, cab1, cab2).
func _on_cabin_occupied_changed(vehicle_rid:RID, _cabin_occupied:int) -> void:
    if not vehicle_rid == get_vehicle_rid():
        return
    if not _select_cab_number() == _last_cab_number:
        _rebuild_generated()


func _select_cab_number() -> int:
    if not get_vehicle_rid():
        return 1
    # Train.cpp:8684 (InitializeCab) - CabOccupied -1 loads cab2definition:, 0 cab0, 1 cab1.
    var cabin_occupied:int = CabinSystem.vehicle_state(get_vehicle_rid()).get("cabin_occupied", 0)
    return 2 if cabin_occupied < 0 else cabin_occupied


func _rebuild_generated() -> void:
    _free_occluders()
    if _generated:
        remove_child(_generated)
        _generated.queue_free()
        _generated = null

    _diagnostics.clear()
    if not mmd_filename or not get_vehicle_rid():
        return

    _last_cab_number = _select_cab_number()
    cab_number = -1 if _last_cab_number == 2 else _last_cab_number

    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var relative_path:String = data_path.trim_prefix("/").path_join(mmd_filename + ".mmd")
    var abs_mmd_path:String = game_dir.path_join(MaszynaDataPath.resolve(game_dir, relative_path))
    var parameters:Dictionary = MmdCabinInstancer.vehicle_parameters(
            VehicleServer.vehicle_get_name(get_vehicle_rid()), mmd_filename, skin)
    var definition:MmdCabinDefinition = MmdCabinInstancer.parse(
            abs_mmd_path, parameters, _last_cab_number, _random_choices)
    _diagnostics.append_array(definition.diagnostics)

    camera_bound_min = definition.bounds_min
    camera_bound_max = definition.bounds_max
    camera_bound_enabled = true
    has_cab_model = true if definition.model_relpath else false
    driver_position = definition.driver_pos
    shake_spring_stiffness = definition.shake_spring_stiffness
    shake_spring_damping = definition.shake_spring_damping
    shake_jolt_scale = definition.shake_jolt_scale
    shake_jolt_limit = definition.shake_jolt_limit
    shake_angle_scale = definition.shake_angle_scale
    engine_shake_scale = definition.engine_shake_scale
    engine_shake_fade_in_rpm = definition.engine_shake_fade_in_rpm
    engine_shake_fade_in_factor = definition.engine_shake_fade_in_factor
    engine_shake_fade_out_rpm = definition.engine_shake_fade_out_rpm
    engine_shake_fade_out_factor = definition.engine_shake_fade_out_factor

    _generated = Node3D.new()
    _generated.name = "Generated"
    add_child(_generated, false, INTERNAL_MODE_BACK)

    var build_diagnostics:Array[Dictionary] = []
    MmdCabinInstancer.build_into(_generated, definition, get_vehicle_rid(), data_path, skin, build_diagnostics)
    _diagnostics.append_array(build_diagnostics)

    var cab_model:E3DModelInstance = _generated.get_node_or_null("CabModel") as E3DModelInstance
    if cab_model:
        if cab_model.is_e3d_loaded():
            _create_occluders(cab_model)
        else:
            cab_model.e3d_loaded.connect(_create_occluders.bind(cab_model), CONNECT_ONE_SHOT)

    # a cab with an i-cablight lamp already has its own ceiling light (MmdSemanticCatalog)
    if not definition.instruments.any(
            func(instrument:MmdInstrumentDescriptor) -> bool: return instrument.label == "i-cablight"):
        _build_cab_light(definition)
    # the vehicle's origin lies on the rail level, so the box is lifted by half of its height; the
    # cab's own turn about the vertical leaves the box as it is
    var rain_exclusion := RainVolume.new()
    rain_exclusion.name = RAIN_EXCLUSION_NAME
    rain_exclusion.precipitation_delta = RAIN_EXCLUSION_PRECIPITATION_DELTA
    rain_exclusion.size = VehicleServer.vehicle_get_dimensions(get_vehicle_rid())
    rain_exclusion.position.y = rain_exclusion.size.y * 0.5
    _generated.add_child(rain_exclusion)
    var windscreen_wipers := CabinWindscreenWipers.new()
    windscreen_wipers.name = "WindscreenWipers"
    windscreen_wipers.vehicle_rid = get_vehicle_rid()
    _generated.add_child(windscreen_wipers)
    camera_configuration_changed.emit()
    _apply_reverse_cull_face()

    print("MaszynaDynamicTrainCabin: built cab %d from %s - %d instruments parsed, %d generated children" % [
        _last_cab_number, abs_mmd_path, definition.instruments.size(), _generated.get_child_count()])
    for d:Dictionary in _diagnostics:
        print("  [%s] %s (label=%s submodel=%s)" % [d["severity"], d["message"], d["mmd_label"], d["submodel_name"]])


## Every light of the cab draws its shadow map with the faces the setting says
## (opengl33renderer.cpp:1758) - once the cab is built, and again when the setting changes
func _apply_reverse_cull_face() -> void:
    if not _generated:
        return
    var reverse:bool = ProjectSettings.get_setting("maszyna/lights/reverse_cull_face", false)
    for light:Node in _generated.find_children("*", "Light3D", true, false):
        (light as Light3D).shadow_reverse_cull_face = reverse


## Every opaque mesh of the cab model hides the controls behind it from the mouse, as the cab is
## drawn into the original's pick buffer without its translucent submodels
## (opengl33renderer.cpp:1208) - E186's spring brake buttons lie under glass caps. Control meshes
## are among them and keep their own hits.
func _create_occluders(cab_model:E3DModelInstance) -> void:
    for mesh_id:int in E3DRenderingServer.instance_get_opaque_meshes(cab_model.get_e3d_instance()):
        _occluders.append(CabinHUDMouseSystem.occluder_create(mesh_id))


func _free_occluders() -> void:
    for occluder:RID in _occluders:
        CabinHUDMouseSystem.occluder_free(occluder)
    _occluders.clear()


## Cab interior lighting: the original lights the cab model with a tungsten ambient term
## InteriorLight * InteriorLightLevel (DynObj.h:240, openglrenderer.cpp:3684-3692); here a shadow
## casting light at the cab ceiling lamp, driven by the same level - the cab light of this cab
## (CabinSystem.cab_light_level_changed, Train.cpp:8436-8453).
func _build_cab_light(definition:MmdCabinDefinition) -> void:
    var light := CabinOmniLight3D.new()
    light.name = "CabLight"
    light.light_color = Color(0.9, 0.9 * 216.0 / 255.0, 0.9 * 176.0 / 255.0)
    light.shadow_enabled = true
    # without a cab model: the top of the camera bounds
    light.position = (definition.bounds_min + definition.bounds_max) * 0.5
    light.position.y = definition.bounds_max.y
    light.omni_range = maxf((definition.bounds_max - definition.bounds_min).length(), 1.0)
    light.cab_light = CabinState.Light.CAB
    _generated.add_child(light)
    light.set_vehicle_rid(get_vehicle_rid())

    var cab_model:E3DModelInstance = _generated.get_node_or_null("CabModel") as E3DModelInstance
    if not cab_model:
        return
    if cab_model.is_e3d_loaded():
        _place_cab_light(light, cab_model, definition.driver_pos.y)
    else:
        cab_model.e3d_loaded.connect(
                _place_cab_light.bind(light, cab_model, definition.driver_pos.y), CONNECT_ONE_SHOT)


## Just under the cab model's ceiling lamp; without one, horizontal center of the cab model above
## the driver's eyes.
func _place_cab_light(light:CabinOmniLight3D, cab_model:E3DModelInstance, driver_height:float) -> void:
    var bounds:AABB = cab_model.submodels_aabb
    light.omni_range = maxf(bounds.size.length(), 1.0)
    var lamp_bounds:AABB = _find_cab_lamp_bounds(cab_model)
    if lamp_bounds.has_volume() or lamp_bounds.has_surface():
        var lamp_center:Vector3 = lamp_bounds.get_center()
        light.position = cab_model.transform * Vector3(
                lamp_center.x, lamp_bounds.position.y - CAB_LIGHT_BELOW_LAMP, lamp_center.z)
        return
    var center:Vector3 = bounds.get_center()
    var height:float = minf(driver_height + CAB_LIGHT_ABOVE_DRIVER, bounds.end.y - CAB_LIGHT_CEILING_OFFSET)
    light.position = cab_model.transform * Vector3(center.x, height, center.z)


## Bounds (in cab model space) of the first ceiling lamp submodel found by CAB_LAMP_SUBMODEL_NAMES,
## matched case-insensitively like TSubModel::GetFromName, also as an _on/_off indicator pair.
func _find_cab_lamp_bounds(cab_model:E3DModelInstance) -> AABB:
    var meshes_by_name:Dictionary = {}
    for mesh:Node in cab_model.find_children("*", "MeshInstance3D", true, false):
        var mesh_name:String = String(mesh.name).to_lower()
        if not meshes_by_name.has(mesh_name):
            meshes_by_name[mesh_name] = mesh
    for lamp_name:String in CAB_LAMP_SUBMODEL_NAMES:
        for suffix:String in ["", "_on", "_off"]:
            var mesh:MeshInstance3D = meshes_by_name.get(lamp_name + suffix) as MeshInstance3D
            if mesh:
                return cab_model.global_transform.affine_inverse() * mesh.global_transform * mesh.get_aabb()
    return AABB()
