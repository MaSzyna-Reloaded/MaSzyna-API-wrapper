@tool
extends ScrollContainer

## The trainsets of the edited scene, each with the side views of its vehicles, listed again
## whenever one of the scene's sceneries has loaded. "Show" selects the trainset's vehicles and
## centres the 3D view on them.

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

    func _init(p_profile:TextureRect, vehicle:MaszynaRailVehicle3D) -> void:
        profile = p_profile
        data_path = vehicle.data_path
        file_name = vehicle.file_name
        skin = vehicle.skin
        vehicle_name = vehicle.vehicle_id


## The sceneries of the edited scene whose `loaded` lists the trainsets again, by instance id - a
## closed scene frees them
var _include_ids:Array[int] = []
var _scene_root:Node = null
## Side views still to render, one after another (a render takes a frame)
var _profile_loads:Array[ProfileLoad] = []
var _loading_profiles:bool = false


## The edited scene: its trainsets are listed now, and again whenever one of its sceneries loads
func set_scene_root(root:Node) -> void:
    for include_id:int in _include_ids:
        var include:MaszynaIncludeNode = instance_from_id(include_id) as MaszynaIncludeNode
        if include:
            include.loaded.disconnect(list_trainsets)
    _include_ids.clear()
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
    for node:Node in _scene_root.find_children("", "TrainSet3D", true, false):
        var row:TrainsetRow = TRAINSET_ROW.instantiate()
        %Trainsets.add_child(row)
        row.set_trainset(node as TrainSet3D)
        row.show_requested.connect(show_trainset)
        for child:Node in node.get_children():
            var vehicle:MaszynaRailVehicle3D = child as MaszynaRailVehicle3D
            if vehicle:
                var tooltip:String = "%s (%s)" % [vehicle.vehicle_id, vehicle.file_name]
                _profile_loads.append(ProfileLoad.new(row.add_vehicle_profile(tooltip), vehicle))
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


func show_trainset(trainset_id:int) -> void:
    var trainset:TrainSet3D = instance_from_id(trainset_id) as TrainSet3D
    if not trainset:
        return
    var selection:EditorSelection = EditorInterface.get_selection()
    selection.clear()
    for child:Node in trainset.get_children():
        if child is RailVehicle3D:
            selection.add_node(child)
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
