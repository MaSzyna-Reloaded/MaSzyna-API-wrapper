@tool
extends Node

## MaSzyna vehicles, built through the servers from what a scenery's `dynamic` says
## (MaszynaDynamicData) - no node per vehicle. vehicle_create() gives the vehicle's handle at once;
## the vehicle itself - its FIZ physics, its appearance (RailVehicleRenderingServer), its cab scene
## (CabinSystem), its sound (TrainSoundSystem) - is built in its turn, a budget a frame, and
## announced by vehicle_built. Whoever created a vehicle frees it (vehicle_free()): a scenery
## (SceneryInstancer), or the node of a vehicle placed by hand (MaszynaRailVehicle3D).
##
## Reading a vehicle's .mmd is several passes over the same file - the body/lowpoly/passengers
## model names, the cab and the sound bank each do their own - and a scenery routinely places
## many vehicles sharing one data_path/file_name/skin (every wagon of one type in a trainset).
## What the MMD says the vehicle is built from (MaszynaVehicleStructure) is cached, which turns an
## O(vehicle count) MMD parse into O(distinct data_path+file_name+skin combinations). What says
## which *instance* a vehicle is - its name, velocity, driver, load - is not in it: two wagons of
## the same type are still two vehicles.

## The vehicle has been built: it has its simulation and is drawn - or neither, when its data
## cannot be read
signal vehicle_built(vehicle:RID)

## Time the vehicle builds may take per frame; a vehicle that started is finished, so a frame
## builds at least one. Built all in one frame, a scenery's vehicles stalled the loading screen; at
## 8 ms - less than one vehicle takes - every frame built one, and a frame of the loading screen
## cost the scenery's drawing on top of each: Wrzosy's 561 vehicles took 370 frames
## (docs/findings-archive.md, 2026-10-03 hundreds of vehicles). A frame at 30 fps.
const BUILD_BUDGET_MSEC:int = 33

## A vehicle of this system, by its handle
class Vehicle:
    var dynamic:MaszynaDynamicData
    ## The scene node whose world the vehicle is drawn in (RailVehicleRenderingServer.vehicle_attach())
    var scene_node_id:int = 0
    var controller:RID = RID()
    var built:bool = false
    ## The node its sound players are built under, riding on the vehicle - there once its sound
    ## is built
    var sound_mount:Node3D = null

## A mirror's glass is a submodel with nothing under it, named after a mirror - the data marks
## mirrors by name only: dynamic/pkp/elf_v1 "zwierciadlo", dynamic/pkp/impuls_v1 "szybka_lusterko_l"
const MIRROR_GLASS_NAME_PARTS:Array[String] = ["zwierciad", "luster", "lustr"]
## Whether the mirror glass reflects the scene (PlanarMirror3D)
const REAL_MIRRORS_SETTING:StringName = &"maszyna/rendering/real_mirrors"

var _cache = ResourceCache.create("rail_vehicle")
var _vehicles:Dictionary[RID, Vehicle] = {}
## Vehicles waiting for their build, in the order they were created; processed while it is not empty
var _build_queue:Array[RID] = []
var _auto_rewident:MaszynaLegacyAutoRewident = null
## REAL_MIRRORS_SETTING as the vehicles' mirrors show it
var _real_mirrors:bool = ProjectSettings.get_setting(REAL_MIRRORS_SETTING, true)


## The editor drives no vehicle and plays no sound: CabinSystem, TrainSoundSystem and DriverSystem's
## drivers are the game's
func _ready() -> void:
    RailVehicleRenderingServer.vehicle_model_built.connect(_on_vehicle_model_built)
    ProjectSettings.settings_changed.connect(_on_project_settings_changed)
    if not Engine.is_editor_hint():
        DriverSystem.vehicle_driven_changed.connect(_on_vehicle_driven_changed)
        _auto_rewident = MaszynaLegacyAutoRewident.new()
        add_child(_auto_rewident)


## A vehicle of `dynamic`, drawn in the world of the scene node `scene_node_id`; built in its turn
func vehicle_create(dynamic:MaszynaDynamicData, scene_node_id:int) -> RID:
    var vehicle:RID = VehicleServer.vehicle_create()
    var record:Vehicle = Vehicle.new()
    record.dynamic = dynamic
    record.scene_node_id = scene_node_id
    record.controller = VehicleServer.controller_create()
    _vehicles[vehicle] = record
    _build_queue.append(vehicle)
    if _build_queue.size() == 1:
        get_tree().process_frame.connect(_on_process_frame)
    return vehicle


## The vehicle goes: its handle, its controller, its sound, its cab logic
func vehicle_free(vehicle:RID) -> void:
    var record:Vehicle = _vehicles[vehicle]
    _vehicles.erase(vehicle)
    if _build_queue.has(vehicle):
        _build_queue.erase(vehicle)
        if not _build_queue:
            get_tree().process_frame.disconnect(_on_process_frame)
    if not Engine.is_editor_hint():
        _auto_rewident.vehicle_unfollow(vehicle)
        TrainSoundSystem.vehicle_set_bank_builder(vehicle, Callable())
        CabinSystem.vehicle_attach_cab_logic(vehicle, null)
    if record.sound_mount:
        record.sound_mount.queue_free()
    VehicleServer.vehicle_free(vehicle)
    VehicleServer.controller_free(record.controller)


## false until the vehicle's turn to be built has come - true then even when it failed to load
func vehicle_is_built(vehicle:RID) -> bool:
    return _vehicles[vehicle].built


## What the vehicle was created from
func vehicle_get_dynamic(vehicle:RID) -> MaszynaDynamicData:
    return _vehicles[vehicle].dynamic


## Whether the vehicle is one of this system's
func vehicle_exists(vehicle:RID) -> bool:
    return _vehicles.has(vehicle)


func clear_cache() -> void:
    _cache.clear()


func _on_process_frame() -> void:
    var deadline:int = Time.get_ticks_msec() + BUILD_BUDGET_MSEC
    while _build_queue and Time.get_ticks_msec() < deadline:
        var vehicle:RID = _build_queue.pop_front()
        _build(vehicle, _vehicles[vehicle])
        vehicle_built.emit(vehicle)
    if not _build_queue:
        get_tree().process_frame.disconnect(_on_process_frame)


## The vehicle's own: the scenery's values first - the controller takes them when its simulation
## starts - then its physics, configured before it is bound, so it is configured once; then what
## it looks like, its cab and its sound, handed to the servers by its handle.
func _build(vehicle:RID, record:Vehicle) -> void:
    record.built = true
    var dynamic:MaszynaDynamicData = record.dynamic
    var structure:MaszynaVehicleStructure = _get_structure(dynamic)
    if not structure:
        return
    VehicleServer.vehicle_set_name(vehicle, dynamic.name)
    VehicleServer.vehicle_set_initial_velocity(vehicle, dynamic.velocity)
    VehicleServer.vehicle_set_driver_type(vehicle, dynamic.driver_type)
    RailVehicleServer.vehicle_attach(vehicle)
    # the original's TypeName is the CHK/MMD name (DynObj.cpp:2019)
    RailVehicleServer.vehicle_set_type_name(vehicle, structure.file_name)
    RailVehicleServer.vehicle_set_load(vehicle, dynamic.load_name, dynamic.load_amount)
    VehicleServer.controller_configure(
            record.controller, FizVehicleBuilder.build_description(structure.data_path, structure.file_name))
    VehicleServer.vehicle_bind_controller(vehicle, record.controller)

    RailVehicleRenderingServer.vehicle_attach(vehicle, record.scene_node_id)
    RailVehicleRenderingServer.vehicle_set_appearance(vehicle, structure.appearance)
    RailVehicleRenderingServer.vehicle_set_load_model(
            vehicle, structure.data_path, MaszynaRailVehicle3DInstancer.load_model_filename(structure, dynamic.load_name))
    MaszynaRailVehicle3DInstancer.apply_wiper_count(vehicle, structure.appearance)
    if Engine.is_editor_hint():
        return
    CabinSystem.vehicle_set_cabin_scene(vehicle, structure.cabin_scene)
    # its sound is built only once it is within earshot - a scenery's vehicles all built at once
    # spent most of their loading on banks nobody hears
    TrainSoundSystem.vehicle_set_bank_builder(vehicle, _build_sounds.bind(vehicle))
    _auto_rewident.vehicle_follow(vehicle)


## The structure of a vehicle type and skin - of the vehicle itself when its MMD names (p1), the
## vehicle's own name (DynObj.cpp:5263) - from the cache, or read and cached; null when the data
## cannot be read
func _get_structure(dynamic:MaszynaDynamicData) -> MaszynaVehicleStructure:
    if not dynamic.data_path or not dynamic.file_name:
        return null
    var normalized_data_path:String = dynamic.data_path if dynamic.data_path.begins_with("/") else "/" + dynamic.data_path
    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var relative_path:String = normalized_data_path.trim_prefix("/").path_join(dynamic.file_name + ".mmd")
    var abs_mmd_path:String = game_dir.path_join(MaszynaDataPath.resolve(game_dir, relative_path))
    var variant:String = dynamic.skin
    if MmdCabinInstancer.names_vehicle(abs_mmd_path):
        variant += "|" + dynamic.name
    var cache_path:String = normalized_data_path.path_join("%s_%s.res" % [dynamic.file_name, variant.md5_text()])
    # Only the .mmd's own mtime is checked - not every .e3d/.fiz file it transitively references -
    # matching FizVehicleBuilder._make_cache_hash()'s same simplification for FIZ `include`s.
    # The hash cannot see changes to MaszynaRailVehicle3DInstancer's own code - bump this tag
    # whenever that code changes the cached structure. v24: the MMD is read with the vehicle's
    # (p1)-(p3), SN61's body model was "none"; attachments.
    var cache_hash:String = ("structure-v24:%s:%s" % [FileAccess.get_modified_time(abs_mmd_path), abs_mmd_path]).md5_text()
    var structure:MaszynaVehicleStructure = _cache.get(cache_path, cache_hash) as MaszynaVehicleStructure
    if not structure:
        structure = MaszynaRailVehicle3DInstancer.read_structure(
                dynamic.data_path, dynamic.file_name, dynamic.skin, dynamic.name)
        if structure:
            _cache.set(cache_path, structure, cache_hash)
    return structure


## The vehicle's sound players, under a node of their own riding on the vehicle - called by
## TrainSoundSystem when the vehicle comes within earshot
func _build_sounds(vehicle:RID) -> void:
    var record:Vehicle = _vehicles[vehicle]
    var dynamic:MaszynaDynamicData = record.dynamic
    record.sound_mount = Node3D.new()
    add_child(record.sound_mount)
    RailVehicleRenderingServer.vehicle_mount_node(vehicle, record.sound_mount.get_instance_id())
    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var relative_mmd_path:String = dynamic.data_path.trim_prefix("/").path_join(dynamic.file_name + ".mmd")
    var abs_mmd_path:String = game_dir.path_join(MaszynaDataPath.resolve(game_dir, relative_mmd_path))
    var diagnostics:Array[Dictionary] = []
    var no_random_choices:Dictionary = {}
    MmdSoundBankInstancer.build_into(
            record.sound_mount, vehicle, abs_mmd_path,
            MmdCabinInstancer.vehicle_parameters(dynamic.name, dynamic.file_name, dynamic.skin), no_random_choices,
            diagnostics)
    for diagnostic:Dictionary in diagnostics:
        if not diagnostic["severity"] == "info":
            push_warning("MaszynaLegacyVehicleSystem: [%s] %s" % [diagnostic["code"], diagnostic["message"]])


## The cab logic is the vehicle's while somebody drives it - its driver or the player - whether a
## 3D cab is shown or not: the AI and the player act on the same controls (CabinSystem). The original
## keeps a TTrain only for a driven train; a cab of every vehicle at work costs every frame.
func _on_vehicle_driven_changed(vehicle:RID, driven:bool) -> void:
    var record:Vehicle = _vehicles.get(vehicle)
    if not record:
        return
    var dynamic:MaszynaDynamicData = record.dynamic
    CabinSystem.vehicle_attach_cab_logic(
            vehicle,
            LegacyCabinLogic.from_mmd(dynamic.data_path, dynamic.file_name, dynamic.skin, dynamic.name) if driven else null)


## The mirrors come and go with their setting on every exterior built as nodes now; the glass of a
## mirror taken away gets its own look back (PlanarMirror3D leaving the tree)
func _on_project_settings_changed() -> void:
    var real_mirrors:bool = ProjectSettings.get_setting(REAL_MIRRORS_SETTING, true)
    if real_mirrors == _real_mirrors:
        return
    _real_mirrors = real_mirrors
    for vehicle:RID in _vehicles:
        var model_root:Node = instance_from_id(E3DRenderingServer.instance_get_attached_node(
                RailVehicleRenderingServer.vehicle_get_model(vehicle))) as Node
        if not model_root:
            continue
        if real_mirrors:
            _add_mirrors(model_root)
            continue
        for mirror:Node in model_root.find_children("*", "PlanarMirror3D", true, false):
            mirror.queue_free()


## The exterior built as nodes - near the camera - gets its mirrors, again every time it is built
func _on_vehicle_model_built(vehicle:RID) -> void:
    if not _vehicles.has(vehicle) or not _real_mirrors:
        return
    var model_root:Node = instance_from_id(E3DRenderingServer.instance_get_attached_node(
            RailVehicleRenderingServer.vehicle_get_model(vehicle))) as Node
    if model_root:
        _add_mirrors(model_root)


## The mirrors' glass reflects the scene: a submodel with nothing under it, named after a mirror,
## gets a PlanarMirror3D - put on the nodes the exterior is built as near the camera (under
## `model_root`)
func _add_mirrors(model_root:Node) -> void:
    for node:Node in model_root.find_children("*", "MeshInstance3D", true, false):
        var glass:MeshInstance3D = node as MeshInstance3D
        var submodel_name:String = glass.name.to_lower()
        if glass.get_child_count(true) == 0 \
                and MIRROR_GLASS_NAME_PARTS.any(func(part:String) -> bool: return submodel_name.contains(part)):
            glass.add_child(PlanarMirror3D.new())
