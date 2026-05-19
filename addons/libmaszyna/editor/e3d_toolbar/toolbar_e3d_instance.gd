@tool
extends HBoxContainer

var _t = 0.0
var _selected_e3d:E3DModelInstance

@onready var btn = $Editable
@onready var _info_window = $InfoWindow

func _ready():
    EditorInterface.get_selection().selection_changed.connect(_on_selection_changed)
    btn.disabled = true
    TrackManager.topology_changed.connect(_on_tracks_topology_changed)
    _on_tracks_topology_changed()

func _exit_tree():
    EditorInterface.get_selection().selection_changed.disconnect(_on_selection_changed)
    TrackManager.topology_changed.disconnect(_on_tracks_topology_changed)

func _find_parent_e3d(node: Node):
    if not node:
        return null

    if node is E3DModelInstance:
        return node
    return _find_parent_e3d(node.get_parent())


func _on_selection_changed():
    var sel:EditorSelection = EditorInterface.get_selection()
    var previously_selected:E3DModelInstance = _selected_e3d

    var nodes = sel.get_selected_nodes()

    _selected_e3d = null
    btn.button_pressed = false

    if nodes.size() == 1:
        var n:Node = nodes[0]
        _selected_e3d = _find_parent_e3d(n)

    btn.disabled = false if _selected_e3d else true
    btn.button_pressed = _selected_e3d and _selected_e3d.editable_in_editor

func _on_editable_toggled(toggled_on):
    if _selected_e3d:
        _selected_e3d.editable_in_editor = toggled_on

func _on_tracks_topology_changed() -> void:
    var btn = $RebuildTopologyButton
    if btn.text.begins_with("*") and not TrackManager.is_topology_changed:
        btn.text = btn.text.substr(2)
    elif not btn.text.begins_with("*") and TrackManager.is_topology_changed:
        btn.text = "* %s" % btn.text

func _on_rebuild_topology_button_pressed() -> void:
    _info_window.show_message("Rebuilding...")
    # Give the UI a frame to show "Rebuilding..." before blocking for rebuild
    await get_tree().process_frame
    TrackManager.rebuild_topology()
    _show_summary()

    _on_tracks_topology_changed()

func _on_view_topology_button_pressed() -> void:
    _show_summary()

func _show_summary() -> void:
    var summary = TrackManager.get_topology_summary()
    var graph_stats = []
    for g in summary.graphs:
        if g.num_tracks > 1:
            graph_stats.append(
                "Graph %d: Tracks: %d, Switches: %d, Length: %.2f" % [
                    g.id, g.num_tracks, g.num_switches, g.total_length
                ])

    var total_orphaned = summary.orphaned_tracks_count

    _info_window.show_message(
        "Tracks topology summary:\n\n" + "\n".join(graph_stats) +
        "\n\nOrphaned tracks: " + str(total_orphaned)
    )
