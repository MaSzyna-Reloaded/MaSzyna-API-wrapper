extends Control

## The loading screen of a scenery: the name large on the left, the stages as bars (the board,
## loading_layout.gd). The screen keeps the time of the load and notices the progress standing;
## the board shows it.

## Seconds the progress may stand before the screen says the load is still going - a large
## include holds the files stage for a while, and a standing bar reads as a hang
const STALL_SEC: float = 1.5
const MSEC_PER_SEC: float = 1000.0
## Height of the window the board is laid out for; in a window of another height it is scaled
## whole, so it keeps its proportions in a resized window as on any screen
const REFERENCE_HEIGHT: float = 1080.0

var _progress: float = 0.0
var _started_msec: int = 0
var _progressed_msec: int = 0


func _ready() -> void:
    visible = false
    # sized and scaled by the window below, not by its anchors
    %Board.set_anchors_preset(Control.PRESET_TOP_LEFT)
    _on_viewport_size_changed()


func _enter_tree() -> void:
    get_viewport().size_changed.connect(_on_viewport_size_changed)


func _exit_tree() -> void:
    get_viewport().size_changed.disconnect(_on_viewport_size_changed)


## "Galicja · Linia 107 Objazdy" (MaszynaSceneryInfo.read_display_name()): the scenery's name,
## then what tells the scenario apart
func show_loading(display_name: String) -> void:
    var parts: PackedStringArray = display_name.split(MaszynaSceneryInfo.PART_SEPARATOR, false)
    %Board.show_title(parts[0] if parts else display_name, MaszynaSceneryInfo.PART_SEPARATOR.join(parts.slice(1)))
    _progress = 0.0
    _started_msec = Time.get_ticks_msec()
    _progressed_msec = _started_msec
    %Board.show_progress(0.0, MaszynaIncludeNode.LoadStage.FILES, "")
    %Board.show_elapsed(0)
    %Board.show_stalled(false)
    visible = true
    %Clock.start()


func set_progress(progress: float, stage: MaszynaIncludeNode.LoadStage, message: String) -> void:
    if progress > _progress:
        _progress = progress
        _progressed_msec = Time.get_ticks_msec()
    %Board.show_progress(progress, stage, message)


func set_files(count: int, filename: String) -> void:
    %Board.show_files(count, filename)


func _on_clock_timeout() -> void:
    var now: int = Time.get_ticks_msec()
    %Board.show_elapsed(floori((now - _started_msec) / MSEC_PER_SEC))
    %Board.show_stalled(now - _progressed_msec > STALL_SEC * MSEC_PER_SEC)


## The board laid out at REFERENCE_HEIGHT, scaled to the window
func _on_viewport_size_changed() -> void:
    var window_size: Vector2 = get_viewport_rect().size
    var factor: float = window_size.y / REFERENCE_HEIGHT
    %Board.scale = Vector2(factor, factor)
    %Board.size = window_size / factor


func _on_visibility_changed() -> void:
    if not visible:
        %Clock.stop()
