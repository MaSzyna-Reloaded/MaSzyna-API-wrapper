@tool
extends RefCounted
class_name MaszynaRailVehicle3DInstancer

## Reads what a vehicle's MMD says it is built from, out of nothing but a
## data_path/file_name/skin triple: its appearance (the models and the submodels that move, drawn
## by RailVehicleRenderingServer), the models of its cargo and its interactive MMD-driven cabin
## (CabinSystem). Used by MaszynaLegacyVehicleSystem, which builds the vehicle from it.

## Fixed original-engine submodel naming convention for bogies/wheel axles (DynObj.cpp:2341-2346
## for bogies, DynObj.cpp:5132-5176 for wheel axles) - confirmed against real game data (e.g.
## dynamic/pkp/su45_v2/301d.e3d, dynamic/pkp/sm42_v1/6d1.e3d both contain bogie1/bogie2
## submodels; every .mmd in the game data declaring animwheelprefix: uses "wheel0").
##
## NOTE: every discovered wheel submodel is treated as powered. The original engine also splits
## axles into front-rolling/powered/rear-rolling groups based on AxleArangement letters/digits
## (DynObj.cpp:5150-5176), but only when a vehicle's rolling-wheel diameter differs from its
## powered-wheel diameter - no vehicle in the current game data needs that, and it would require
## threading the parsed RailVehicleWheels config through to spawn time. Left unimplemented until an
## actual vehicle needs it.
const FRONT_BOGIE_SUBMODEL_NAMES:Array[String] = ["bogie1", "boogie01"]
const REAR_BOGIE_SUBMODEL_NAMES:Array[String] = ["bogie2", "boogie02"]
const WHEEL_SUBMODEL_PREFIX:String = "wheel0"
const WIPER_ELEMENT_SUFFIXES:Array[String] = ["_p1", "_p2", "_p3"]
## std::vector<bool>(8, false) wiperDirection of the original (DynObj.h:328)
const MAX_WIPERS:int = 8
const MAX_WHEEL_AXLES:int = 20
## A mirror's glass is a submodel with nothing under it, named after a mirror - the data marks
## mirrors by name only: dynamic/pkp/elf_v1 "zwierciadlo", dynamic/pkp/impuls_v1 "szybka_lusterko_l"
const MIRROR_GLASS_NAME_PARTS:Array[String] = ["zwierciad", "luster", "lustr"]
## Whether the mirror glass reflects the scene (PlanarMirror3D)
const REAL_MIRRORS_SETTING:StringName = &"maszyna/rendering/real_mirrors"

## lower arm 1, upper arm 1 and the slider
const PANTOGRAPH_REQUIRED_ARMS:Array[int] = [0, 2, 4]
const PANTOGRAPH_ARM_SUBMODEL_PREFIXES:Array[String] = [
    "ramiedolne1_pant0", "ramiedolne2_pant0", "ramiegorne1_pant0", "ramiegorne2_pant0", "slizg_pant0",
]

## Every MaSzyna-authored piece of a vehicle (exterior, low-poly interior, passengers, cab) lives in
## one vehicle-local frame where +Z is the direction of travel: the original draws all of them under
## the same TDynamicObject::mMatrix, built by BasisChange(vLeft, vUp, vFront) (DynObj.cpp:2506-2508;
## opengl33renderer.cpp:1174, 2856, 2976). A vehicle faces -Z here, so each of them is turned about
## its vertical by half a turn.
const MASZYNA_VEHICLE_FRAME:Transform3D = Transform3D(Basis(Vector3.UP, PI), Vector3.ZERO)


## Reads what the vehicle's MMD says it is built from. This is the expensive half - every call
## opens and re-parses the MMD and loads the exterior model - and its result is what
## MaszynaLegacyVehicleSystem caches.
static func read_structure(
        data_path:String, file_name:String, skin:String, vehicle_name:String) -> MaszynaVehicleStructure:
    if not data_path or not file_name:
        return null

    # MaterialManager.get_submodel_material() builds its material-search path by dropping
    # data_path's FIRST "/"-separated segment - meant to strip the artifact empty segment from a
    # LEADING slash, not the "dynamic" directory name itself. Every hand-authored vehicle scene
    # except su45 (whose skin is consequently broken the same way) uses a leading slash
    # (e.g. "/dynamic/pkp/ep09_v1/") for exactly this reason. Normalize here so operators don't
    # need to know about this quirk.
    var normalized_data_path:String = data_path if data_path.begins_with("/") else "/" + data_path
    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var relative_mmd_path:String = normalized_data_path.trim_prefix("/").path_join(file_name + ".mmd")
    var abs_mmd_path:String = game_dir.path_join(MaszynaDataPath.resolve(game_dir, relative_mmd_path))
    var parameters:Dictionary = MmdCabinInstancer.vehicle_parameters(vehicle_name, file_name, skin)

    var structure := MaszynaVehicleStructure.new()
    structure.data_path = normalized_data_path
    structure.file_name = file_name
    var appearance := RailVehicleAppearance.new()
    appearance.data_path = normalized_data_path
    appearance.model_transform = MASZYNA_VEHICLE_FRAME
    # The exterior body model filename is NOT the same as file_name in general (confirmed
    # against real data: dynamic/pkp/st44_v2's body model isn't named after its .fiz/.mmd base) -
    # it comes from the MMD's own top-level "models:" line. Fall back to file_name only if that
    # can't be read, rather than silently building a vehicle with no model at all.
    var body_model_filename:String = MmdCabinInstancer.parse_body_model(abs_mmd_path, parameters)
    if not body_model_filename:
        body_model_filename = file_name
    appearance.model_filename = body_model_filename

    var lowpoly_filename:String = MmdCabinInstancer.parse_lowpoly_interior_model(abs_mmd_path, parameters)
    if lowpoly_filename:
        appearance.low_poly_model_filename = lowpoly_filename

    appearance.attachment_model_filenames = MmdCabinInstancer.parse_attachments(abs_mmd_path, parameters)

    structure.load_models = MmdCabinInstancer.parse_loads(abs_mmd_path, parameters)
    var passengers_filename:String = structure.load_models.get("passengers", "")
    if passengers_filename:
        appearance.passengers_model_filename = passengers_filename

    appearance.skins = PackedStringArray(MmdCabinInstancer.resolve_skins(normalized_data_path, skin))
    appearance.joint_cabs = MmdCabinInstancer.parse_joint_cabs(abs_mmd_path, parameters)
    var model:E3DModel = E3DModelManager.load_model(normalized_data_path, appearance.model_filename)
    if model:
        _resolve_parts(appearance, model, MmdCabinInstancer.parse_wiper_prefix(abs_mmd_path, parameters),
                MmdCabinInstancer.parse_mirror_names(abs_mmd_path, parameters))
    structure.appearance = appearance
    structure.cabin_scene = _build_cabin_scene(normalized_data_path, file_name, skin)
    return structure


## Which model a cargo is drawn as, in the order the original tries them
## (TDynamicObject::LoadMMediaFile_mdload(), DynObj.cpp:7195): the vehicle's own override for that
## cargo, then a model named for this vehicle and the cargo together, then one named after the
## cargo alone. Empty when the cargo has no model anywhere, which is not an error - plenty of
## loads are only mass.
static func load_model_filename(structure:MaszynaVehicleStructure, load_name:String) -> String:
    if not load_name:
        return ""
    var override:String = structure.load_models.get(load_name.to_lower(), "")
    if override:
        return override
    var specialized:String = "%s_%s" % [structure.file_name, load_name]
    if _model_exists(structure.data_path, specialized):
        return specialized
    var generic:String = load_name
    return generic if _model_exists(structure.data_path, generic) else ""


static func _model_exists(data_path:String, relpath:String) -> bool:
    if not relpath:
        return false
    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var relative_base_path:String = data_path.trim_prefix("/").path_join(relpath)
    var e3d_path:String = MaszynaDataPath.resolve(game_dir, relative_base_path + ".e3d")
    if FileAccess.file_exists(game_dir.path_join(e3d_path)):
        return true
    var t3d_path:String = MaszynaDataPath.resolve(game_dir, relative_base_path + ".t3d")
    return FileAccess.file_exists(game_dir.path_join(t3d_path))


## The mirrors' glass reflects the scene (maszyna/rendering/real_mirrors): a submodel with nothing
## under it, named after a mirror, gets a PlanarMirror3D - put on the nodes the exterior is built
## as near the camera (under `model_root`), so again every time it is built
## (RailVehicleRenderingServer.vehicle_model_built)
static func add_mirrors(model_root:Node) -> void:
    if not ProjectSettings.get_setting(REAL_MIRRORS_SETTING, true):
        return
    for node:Node in model_root.find_children("*", "MeshInstance3D", true, false):
        var glass:MeshInstance3D = node as MeshInstance3D
        var submodel_name:String = glass.name.to_lower()
        if glass.get_child_count(true) == 0 \
                and MIRROR_GLASS_NAME_PARTS.any(func(part:String) -> bool: return submodel_name.contains(part)):
            glass.add_child(PlanarMirror3D.new())


## PackedScene.pack()-in-memory trick, already used in production by
## FizVehicleBuilder.build_scene() - CabinSystem.vehicle_show_cabin() instantiates a correctly
## pre-configured MaszynaDynamicTrainCabin every time. The cab is drawn in the MaSzyna vehicle frame
## like the models (MASZYNA_VEHICLE_FRAME).
static func _build_cabin_scene(normalized_data_path:String, file_name:String, skin:String) -> PackedScene:
    var cabin := MaszynaDynamicTrainCabin.new()
    cabin.data_path = normalized_data_path
    cabin.mmd_filename = file_name
    cabin.skin = skin
    cabin.transform = MASZYNA_VEHICLE_FRAME
    var packed := PackedScene.new()
    var err:Error = packed.pack(cabin)
    cabin.free()
    if err != OK:
        push_error("MaszynaRailVehicle3DInstancer: could not pack cabin scene for %s/%s" % [normalized_data_path, file_name])
        return null
    return packed


## The submodels of the exterior model that move, by the original's names.
static func _resolve_parts(
        appearance:RailVehicleAppearance, model:E3DModel, wiper_prefix:String,
        mirror_names:PackedStringArray) -> void:
    var names:Dictionary[String, bool] = {}
    _index_submodels(model.submodels, names)

    appearance.front_bogie = _find_submodel(names, FRONT_BOGIE_SUBMODEL_NAMES)
    appearance.rear_bogie = _find_submodel(names, REAR_BOGIE_SUBMODEL_NAMES)

    var powered_wheels:PackedStringArray = []
    for axle_index:int in range(1, MAX_WHEEL_AXLES + 1):
        var wheel:String = _find_submodel(names, ["%s%d" % [WHEEL_SUBMODEL_PREFIX, axle_index]])
        if wheel:
            powered_wheels.append(wheel)
    appearance.powered_wheels = powered_wheels

    appearance.pantograph_front_arms = _find_pantograph_arms(names, 1)
    appearance.pantograph_rear_arms = _find_pantograph_arms(names, 2)
    if wiper_prefix:
        appearance.wiper_arms = _find_wiper_arms(names, wiper_prefix)
    var mirrors:PackedStringArray = []
    for mirror_name:String in mirror_names:
        mirrors.append(_find_submodel(names, [mirror_name.to_lower()]))
    appearance.mirrors = mirrors


## The 5 arm/slider names for the given pantograph number (1=front, 2=rear), or none when the lower
## arm 1, the upper arm 1 or the slider is missing - RailVehicleRenderingServer takes the geometry
## of the pantograph from these three. The second arm of each pair is optional, an empty name
## without it: a single-arm pantograph has none (dynamic/pkp/e186_v2 has no "ramiegorne2"), and the
## original does not animate a missing element either (DynObj.cpp:5414).
static func _find_pantograph_arms(names:Dictionary[String, bool], pantograph_number:int) -> PackedStringArray:
    var arms:PackedStringArray = []
    for index:int in PANTOGRAPH_ARM_SUBMODEL_PREFIXES.size():
        var arm:String = _find_submodel(names, ["%s%d" % [PANTOGRAPH_ARM_SUBMODEL_PREFIXES[index], pantograph_number]])
        if not arm and index in PANTOGRAPH_REQUIRED_ARMS:
            return PackedStringArray()
        arms.append(arm)
    return arms


## RailVehicleWipers has to know how many wipers the model has: from cab 2 they are numbered from the
## other end (DynObj.cpp:4062).
static func apply_wiper_count(vehicle:RID, appearance:RailVehicleAppearance) -> void:
    var wipers:RailVehicleWipers = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_WIPERS) as RailVehicleWipers
    if wipers:
        wipers.wiper_count = appearance.wiper_arms.size() / WIPER_ELEMENT_SUFFIXES.size()
        wipers.apply_config()


## Arm 1, arm 2 and blade of every wiper - "<prefix><number>_p1/_p2/_p3", numbered from 1
## (DynObj.cpp:5838-5870); an empty name for an element the model does not have. The original
## takes the number of wipers from the MMD "animations:" counts, here they are collected until a
## wiper with no element at all.
static func _find_wiper_arms(names:Dictionary[String, bool], wiper_prefix:String) -> PackedStringArray:
    var arms:PackedStringArray = []
    for wiper:int in range(1, MAX_WIPERS + 1):
        var wiper_arms:PackedStringArray = []
        for element:String in WIPER_ELEMENT_SUFFIXES:
            wiper_arms.append(_find_submodel(names, ["%s%d%s" % [wiper_prefix.to_lower(), wiper, element]]))
        if not Array(wiper_arms).any(func(arm:String) -> bool: return not arm == ""):
            break
        arms.append_array(wiper_arms)
    return arms


static func _find_submodel(names:Dictionary[String, bool], candidates:Array[String]) -> String:
    for candidate:String in candidates:
        if names.has(candidate):
            return candidate
    return ""


## Indexed by LOWERCASED name, matching the original engine's own TSubModel::GetFromName
## (case-insensitive by default) - see mmd_cabin_instancer.gd's _index_submodels for the
## real-data case mismatch (su45_v2) this guards against.
static func _index_submodels(submodels:Array, names:Dictionary[String, bool]) -> void:
    for item:Variant in submodels:
        var submodel:E3DSubModel = item as E3DSubModel
        if submodel:
            names[submodel.resource_name.to_lower()] = true
            _index_submodels(submodel.submodels, names)
