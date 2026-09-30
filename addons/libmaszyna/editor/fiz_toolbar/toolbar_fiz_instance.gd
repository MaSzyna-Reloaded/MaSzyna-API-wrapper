@tool
extends HBoxContainer

## "Edit FIZ": shows the vehicle a MaszynaRailVehicle3D builds as internal children in the Scene
## dock, and hides it again. That node is the only one with something to show - a
## FizVehiclePhysicsNode has no node tree of its own (a .fiz is data, the vehicle's description).

var _selected_vehicle:MaszynaRailVehicle3D

@onready var btn = $Editable

func _ready():
    EditorInterface.get_selection().selection_changed.connect(_on_selection_changed)
    btn.disabled = true

func _exit_tree():
    EditorInterface.get_selection().selection_changed.disconnect(_on_selection_changed)


## The MaszynaRailVehicle3D the node is, or stands under - once shown, the vehicle it built can
## itself be selected, and has to be hidden from there
func _find_vehicle(node:Node) -> MaszynaRailVehicle3D:
    if not node:
        return null
    if node is MaszynaRailVehicle3D:
        return node
    return _find_vehicle(node.get_parent())


func _on_selection_changed():
    var nodes:Array[Node] = EditorInterface.get_selection().get_selected_nodes()
    _selected_vehicle = _find_vehicle(nodes[0]) if nodes.size() == 1 else null
    btn.disabled = not _selected_vehicle
    # shown, not switched: showing the state must not toggle it
    btn.set_pressed_no_signal(_selected_vehicle and _selected_vehicle.editable_in_editor)

func _on_editable_toggled(toggled_on):
    if not _selected_vehicle:
        return
    _selected_vehicle.editable_in_editor = toggled_on
