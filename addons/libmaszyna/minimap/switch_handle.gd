@tool
extends Control

var track_rid: RID
var viewer: Control
var _hovering: bool = false

func _draw() -> void:
    var radius = size.x * 0.5
    var color = Color.WHITE
    if _hovering:
        color = Color.YELLOW

    # Draw a circle with a border
    draw_circle(Vector2(radius, radius), radius, color)
    draw_arc(Vector2(radius, radius), radius, 0, TAU, 32, Color.BLACK, 1.0)

func _notification(what: int) -> void:
    match what:
        NOTIFICATION_MOUSE_ENTER:
            _hovering = true
            queue_redraw()
        NOTIFICATION_MOUSE_EXIT:
            _hovering = false
            queue_redraw()

func _gui_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        if event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
            _toggle_switch()
            accept_event()

func _toggle_switch() -> void:
    if not TrackManager:
        return
    var track = TrackManager.get_track(track_rid)
    if not track:
        return

    var new_state = 1 if track.switch_state == 0 else 0
    TrackManager.set_switch_state(track_rid, new_state)
    viewer.queue_redraw()

func _get_tooltip(_at_position: Vector2) -> String:
    if not TrackManager:
        return ""
    var track = TrackManager.get_track(track_rid)
    if not track:
        return ""
    var state_str = "Straight" if track.switch_state == 0 else "Diverging"
    return "Switch: %s\nState: %s" % [track.name if track.name else "unnamed", state_str]
