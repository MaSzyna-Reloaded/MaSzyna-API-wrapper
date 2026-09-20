extends VBoxContainer

## Live view of SceneryStreamingServer: what a scenery registered, what is actually built around
## the camera right now, and how much of the plan is still waiting for the per-frame budget.

## Seconds between refreshes - the numbers only change with a streaming pass
const REFRESH_INTERVAL:float = 0.25

var _rows:Dictionary[String, Label] = {}
var _elapsed:float = 0.0


func _ready() -> void:
    for caption:String in [
        "Camera", "Camera chunk", "Draw distance", "Chunks", "Chunks in range",
        "Registered", "Streamed in", "Pending builds", "Pending nearby", "Nearby ready",
        "Pending clears", "Planning", "Builds/s", "Budget", "Last pass", "Owners",
    ]:
        _rows[caption] = _add_row(caption)
    _refresh()


func _process(delta:float) -> void:
    _elapsed += delta
    if _elapsed < REFRESH_INTERVAL:
        return
    _elapsed = 0.0
    _refresh()


func _refresh() -> void:
    var statistics:Dictionary = SceneryStreamingServer.get_statistics()
    var camera_position:Vector3 = statistics["camera_position"]
    var camera_chunk:Vector2i = statistics["camera_chunk"]
    var registered:int = statistics["registered"]
    var streamed:int = statistics["streamed"]

    _rows["Camera"].text = (
        "%.0f, %.0f, %.0f" % [camera_position.x, camera_position.y, camera_position.z]
        if statistics["has_camera"] else "none - nothing is streamed"
    )
    _rows["Camera chunk"].text = "%d, %d" % [camera_chunk.x, camera_chunk.y]
    _rows["Draw distance"].text = "%.0f m (chunk %.0f m)" % [statistics["draw_distance"], statistics["chunk_size"]]
    _rows["Chunks"].text = str(statistics["chunks"])
    _rows["Chunks in range"].text = str(statistics["active_chunks"])
    _rows["Registered"].text = str(registered)
    _rows["Streamed in"].text = (
        "%d (%.1f%%)" % [streamed, 100.0 * float(streamed) / float(registered)] if registered
        else str(streamed)
    )
    _rows["Pending builds"].text = str(statistics["pending_builds"])
    _rows["Pending nearby"].text = str(statistics["pending_nearby"])
    _rows["Nearby ready"].text = str(statistics["nearby_ready"])
    _rows["Pending clears"].text = str(statistics["pending_clears"])
    _rows["Planning"].text = str(statistics["planning"])
    _rows["Builds/s"].text = str(statistics["build_rate"])
    _rows["Budget"].text = "%d ms/frame" % statistics["budget_msec"]
    _rows["Last pass"].text = "%d ms" % statistics["plan_msec"]
    _rows["Owners"].text = str(statistics["owners"])


func _add_row(caption:String) -> Label:
    var row := HBoxContainer.new()
    var caption_label := Label.new()
    caption_label.text = caption
    caption_label.custom_minimum_size = Vector2(130, 0)
    var value_label := Label.new()
    value_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    value_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    row.add_child(caption_label)
    row.add_child(value_label)
    add_child(row)
    return value_label
