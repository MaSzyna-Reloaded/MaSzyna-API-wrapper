@tool
extends Node
class_name FIZTrainController

## Live/no-import FIZ vehicle loader, analogous to E3DModelInstance for E3D models: set
## `data_path`/`fiz_filename` and this node (re)builds a child TrainController + its
## TrainPart children from that file via FizTrainControllerInstancer, in-editor and at
## runtime, without any import step. Point a RailVehicle3D.controller_path at the generated
## child TrainController, the same way RailVehicle3D.model_instance_path points at a separate
## E3DModelInstance.
##
## Deliberately does NOT extend TrainController: TrainController's own exported properties
## (mass, power, dimensions, ...) get serialized into the .tscn on save, which would directly
## conflict with them also being derived fresh from the FIZ file on every load - the node
## holding data_path/fiz_filename and the node holding the FIZ-derived TrainController state
## must stay separate, exactly like E3DModelInstance (VisualInstance3D) never inherits from
## E3DModel itself.

## Base MaSzyna data path used to resolve the FIZ file, matching E3DModelInstance.data_path.
@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _request_reload()

## FIZ file name, without the ".fiz" extension, matching E3DModelInstance.model_filename.
@export var fiz_filename:String = "":
    set(x):
        if not x == fiz_filename:
            fiz_filename = x
            _request_reload()

## When false (default), the generated TrainController subtree is added as INTERNAL children:
## hidden from the Scene dock and excluded from scene serialization, so it never gets baked
## into the .tscn (it's re-derived from the FIZ file on every load instead). Toggle via the
## "Edit FIZ" 3D-viewport toolbar button (see addons/libmaszyna/editor/fiz_toolbar/) to make
## the subtree visible/selectable for inspection or one-off manual tweaks - mirrors
## E3DModelInstance.editable_in_editor exactly.
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _request_reload()

var _reload_pending:bool = false
var _controller:TrainController = null


func get_controller() -> TrainController:
    return _controller


func _ready() -> void:
    _request_reload()


func _request_reload() -> void:
    if _reload_pending:
        return
    _reload_pending = true
    call_deferred("_reload")


func _reload() -> void:
    _reload_pending = false

    if _controller:
        remove_child(_controller)
        _controller.queue_free()
        _controller = null

    if not fiz_filename or not is_inside_tree():
        return

    var abs_fiz_path: String = (
            UserSettings.get_maszyna_game_dir().path_join(data_path).path_join(fiz_filename + ".fiz"))
    _controller = FizTrainControllerInstancer.build(abs_fiz_path)

    var internal_mode: int = INTERNAL_MODE_DISABLED if editable_in_editor else INTERNAL_MODE_BACK
    add_child(_controller, false, internal_mode)
    # Mirrors e3d_nodes_instancer.gd's owner rule exactly: internal (non-editable) children
    # are owned by `self` (excluded from serialization regardless, since they're internal);
    # editable children are owned by `self`'s own owner, i.e. the actual scene root, so they
    # show up in the Scene dock and survive scene serialization.
    _set_owner_recursive(_controller, owner if editable_in_editor else self)


func _set_owner_recursive(node: Node, target_owner: Node) -> void:
    node.owner = target_owner
    for child: Node in node.get_children():
        _set_owner_recursive(child, target_owner)
