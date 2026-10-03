@tool
extends EditorPlugin

## The vehicles of the edited scene - a scenery's trainsets - in a bottom panel tab, next to
## Scenery Streaming, with the side views of their vehicles; "Show" takes the 3D view to one.

const TrainsetsDock = preload("./trainsets_dock.gd")
const TRAINSETS_DOCK:PackedScene = preload("./trainsets_dock.tscn")

var _trainsets_dock:TrainsetsDock = null


func _enter_tree() -> void:
    _trainsets_dock = TRAINSETS_DOCK.instantiate()
    add_control_to_bottom_panel(_trainsets_dock, "Trainsets")
    scene_changed.connect(_trainsets_dock.set_scene_root)
    _trainsets_dock.set_scene_root(EditorInterface.get_edited_scene_root())


func _exit_tree() -> void:
    scene_changed.disconnect(_trainsets_dock.set_scene_root)
    remove_control_from_bottom_panel(_trainsets_dock)
    _trainsets_dock.free()
    _trainsets_dock = null
