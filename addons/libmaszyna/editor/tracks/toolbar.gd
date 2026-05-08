@tool
extends HBoxContainer


@onready var _info_window = $InfoWindow


func _ready() -> void:
    TrackManager.topology_changed.connect(_on_tracks_topology_changed)
    _on_tracks_topology_changed()

func _exit_tree() -> void:
    TrackManager.topology_changed.disconnect(_on_tracks_topology_changed)

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
    TrackManager.topology_rebuild()
    _show_summary()

    _on_tracks_topology_changed()

func _on_view_topology_button_pressed() -> void:
    _show_summary()

func _show_summary() -> void:
    var summary = TrackManager.topology_get_summary()
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
