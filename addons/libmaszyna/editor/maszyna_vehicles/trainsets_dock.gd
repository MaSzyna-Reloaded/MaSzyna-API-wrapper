@tool
extends ScrollContainer

## The trainsets of the edited scene - those its sceneries placed (RailVehicleServer's, by their
## handles) and those assembled by hand (TrainSet3D) - each with the side views of its vehicles,
## listed again whenever one of the scene's sceneries has loaded. "Show" centres the 3D view on the
## trainset's vehicles. A scenery's `dynamic` outside a `trainset` has no name and is not listed.

const TrainsetRow = preload("./trainset_row.gd")
const TRAINSET_ROW:PackedScene = preload("./trainset_row.tscn")
## The editor shortcut of the 3D view's "Focus Selection" (node_3d_editor_plugin.cpp:10692)
const FOCUS_SELECTION_SHORTCUT:String = "spatial_editor/focus_selection"


## A vehicle's side view to render, and the place it goes to - gone when the list is built again
class ProfileLoad:
    var profile:TextureRect
    var data_path:String
    var file_name:String
    var skin:String
    var vehicle_name:String

    func _init(p_profile:TextureRect, dynamic:MaszynaDynamicData) -> void:
        profile = p_profile
        data_path = dynamic.data_path
        file_name = dynamic.file_name
        skin = dynamic.skin
        vehicle_name = dynamic.name


## The sceneries of the edited scene whose `loaded` lists the trainsets again, by instance id - a
## closed scene frees them
var _include_ids:Array[int] = []
var _scene_root:Node = null
## Side views still to render, one after another (a render takes a frame)
var _profile_loads:Array[ProfileLoad] = []
var _loading_profiles:bool = false
## The vehicles have no nodes to select, and the 3D view goes only to what is selected: "Show" puts
## a marker node where each vehicle of the trainset stands, selects those, and takes them away at
## the next "Show" or another scene
var _show_markers:Array[Node3D] = []


## The edited scene: its trainsets are listed now, and again whenever one of its sceneries loads
func set_scene_root(root:Node) -> void:
    for include_id:int in _include_ids:
        var include:MaszynaIncludeNode = instance_from_id(include_id) as MaszynaIncludeNode
        if include:
            include.loaded.disconnect(list_trainsets)
    _include_ids.clear()
    _free_show_markers()
    _scene_root = root
    if root:
        # the sceneries the scene itself holds; the includes of a scenery are its content
        var includes:Array[Node] = root.find_children("", "MaszynaIncludeNode", true, true)
        if root is MaszynaIncludeNode:
            includes.append(root)
        for node:Node in includes:
            (node as MaszynaIncludeNode).loaded.connect(list_trainsets)
            _include_ids.append(node.get_instance_id())
    list_trainsets()


func list_trainsets() -> void:
    for row:Node in %Trainsets.get_children():
        row.queue_free()
    _profile_loads.clear()
    if not _scene_root:
        return
    var trainsets:Array[RID] = []
    for node:Node in _scene_root.find_children("", "TrainSet3D", true, false):
        trainsets.append((node as TrainSet3D).get_rid())
    for include_id:int in _include_ids:
        trainsets.append_array((instance_from_id(include_id) as MaszynaIncludeNode).get_trainsets())
    for trainset:RID in trainsets:
        var trainset_name:String = RailVehicleServer.trainset_get_name(trainset)
        if not trainset_name:
            continue
        var vehicles:Array[RID] = []
        vehicles.assign(RailVehicleServer.trainset_get_vehicles(trainset))
        var row:TrainsetRow = TRAINSET_ROW.instantiate()
        %Trainsets.add_child(row)
        row.set_trainset(trainset_name, vehicles)
        row.show_requested.connect(show_trainset)
        for vehicle:RID in vehicles:
            if MaszynaLegacyVehicleSystem.vehicle_exists(vehicle):
                var dynamic:MaszynaDynamicData = MaszynaLegacyVehicleSystem.vehicle_get_dynamic(vehicle)
                var tooltip:String = "%s (%s)" % [dynamic.name, dynamic.file_name]
                _profile_loads.append(ProfileLoad.new(row.add_vehicle_profile(tooltip), dynamic))
    if not _loading_profiles:
        _load_profiles()


## The side views not rendered yet go to their places; a place freed meanwhile takes nothing
func _load_profiles() -> void:
    _loading_profiles = true
    while _profile_loads:
        var profile_load:ProfileLoad = _profile_loads.pop_front()
        var texture:Texture2D = await MaszynaVehicleProfileManager.get_profile(
                profile_load.data_path, profile_load.file_name, profile_load.skin, profile_load.vehicle_name)
        if texture and is_instance_valid(profile_load.profile):
            TrainsetRow.show_profile(profile_load.profile, texture)
    _loading_profiles = false


func show_trainset(vehicles:Array[RID]) -> void:
    _free_show_markers()
    var selection:EditorSelection = EditorInterface.get_selection()
    selection.clear()
    for vehicle:RID in vehicles:
        var marker:Node3D = Node3D.new()
        _scene_root.add_child(marker, false, Node.INTERNAL_MODE_BACK)
        marker.global_transform = RailVehicleRenderingServer.vehicle_get_transform(vehicle)
        _show_markers.append(marker)
        selection.add_node(marker)
    EditorInterface.set_main_screen_editor("3D")
    # The 3D view has no API for its camera. What moves it is its own "Focus Selection"
    # (Node3DEditorViewport::focus_selection(), node_3d_editor_plugin.cpp:4421), an item of the
    # view's menu - found by the editor shortcut it carries (:6900), not by its translated text.
    # The menu is in the Node3DEditorViewport around the view's SubViewportContainer.
    var focus:Shortcut = EditorInterface.get_editor_settings().get_shortcut(FOCUS_SELECTION_SHORTCUT)
    var viewport_editor:Node = EditorInterface.get_editor_viewport_3d(0).get_parent().get_parent()
    for node:Node in viewport_editor.find_children("", "MenuButton", true, false):
        var menu:PopupMenu = (node as MenuButton).get_popup()
        for index:int in menu.item_count:
            if menu.get_item_shortcut(index) == focus:
                menu.id_pressed.emit(menu.get_item_id(index))
                return


func _free_show_markers() -> void:
    for marker:Node3D in _show_markers:
        if is_instance_valid(marker):
            marker.queue_free()
    _show_markers.clear()
