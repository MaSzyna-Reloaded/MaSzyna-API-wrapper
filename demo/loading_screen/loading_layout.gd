class_name LoadingLayout
extends Control

## What the loading screen shows (loading_board.tscn): the scenery's name, a ring with the progress
## and a comet that keeps turning, the stages of the load (MaszynaIncludeNode.LoadStage) as bars,
## the files parsed, the time and, once the progress stands, that the load is still going. The
## screen (loading_screen.gd) decides what is shown; this only shows it.

## Where the load is with a stage
enum StageState { TO_COME, DONE, UNDER_WAY }

## Names of MaszynaIncludeNode.LoadStage, in its order
const STAGE_NAMES: Array[String] = [
    "Scenery files", "Infrastructure", "Terrain", "Objects", "Vehicles", "Surroundings"
]
const STAGE_SHADER: Shader = preload("loading_stage.gdshader")
## A stage's name in each StageState
const STAGE_COLORS: Array[Color] = [Color(0.81, 0.88, 1.0, 0.4), Color(0.75, 0.82, 0.92, 1.0), Color(1.0, 1.0, 1.0, 1.0)]
const STAGE_FONT_SIZE: int = 15
const BAR_HEIGHT: float = 6.0
const STAGE_SEPARATION: int = 10
## Seconds of a cycle of the "still working" pulse
const STALLED_PULSE_SEC: float = 1.2
const STALLED_PULSE_MIN_ALPHA: float = 0.35

## The indicator of each stage, in the order of MaszynaIncludeNode.LoadStage
var _stage_indicators: Array[ColorRect] = []
var _stage_labels: Array[Label] = []
var _pulse: Tween = null


func _ready() -> void:
    for stage_name: String in STAGE_NAMES:
        var row := VBoxContainer.new()
        row.add_theme_constant_override("separation", STAGE_SEPARATION)
        row.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        var indicator := ColorRect.new()
        indicator.material = ShaderMaterial.new()
        (indicator.material as ShaderMaterial).shader = STAGE_SHADER
        indicator.custom_minimum_size = Vector2(0.0, BAR_HEIGHT)
        indicator.size_flags_vertical = Control.SIZE_SHRINK_CENTER
        var label := Label.new()
        label.text = stage_name
        label.add_theme_font_size_override("font_size", STAGE_FONT_SIZE)
        label.text_overrun_behavior = TextServer.OVERRUN_TRIM_ELLIPSIS
        row.add_child(indicator)
        row.add_child(label)
        %Stages.add_child(row)
        _stage_indicators.append(indicator)
        _stage_labels.append(label)
    show_progress(0.0, MaszynaIncludeNode.LoadStage.FILES, "")


## "Galicja · Linia 107 Objazdy": the scenery's own name, and what tells this scenario apart
func show_title(scenery: String, scenario: String) -> void:
    %Scenery.text = scenery
    %Scenario.text = scenario
    %Scenario.visible = not scenario == ""


func show_progress(progress: float, stage: MaszynaIncludeNode.LoadStage, message: String) -> void:
    (%Ring.material as ShaderMaterial).set_shader_parameter("progress", progress)
    %Percent.text = "%d%%" % roundi(progress * 100.0)
    for index: int in _stage_indicators.size():
        var material: ShaderMaterial = _stage_indicators[index].material as ShaderMaterial
        material.set_shader_parameter("done", 1.0 if index < stage or progress >= 1.0 else 0.0)
        material.set_shader_parameter("active", 1.0 if index == stage and progress < 1.0 else 0.0)
        var state: StageState = StageState.DONE if index < stage else (
            StageState.UNDER_WAY if index == stage else StageState.TO_COME
        )
        _stage_labels[index].add_theme_color_override("font_color", STAGE_COLORS[state])
    # while the files are parsed the detail is the file (show_files()), after that what is done
    if not stage == MaszynaIncludeNode.LoadStage.FILES:
        %Detail.text = message
    %Files.visible = stage == MaszynaIncludeNode.LoadStage.FILES


## The includes parsed so far and the file being parsed now
func show_files(count: int, filename: String) -> void:
    %Files.text = tr("Files %d") % count
    %Detail.text = "› %s" % filename if filename else ""


func show_elapsed(seconds: int) -> void:
    %Elapsed.text = tr("Time %d:%02d") % [floori(seconds / 60.0), seconds % 60]


## The progress has stood a while, yet the load goes on: a pulsing "still working"
func show_stalled(stalled: bool) -> void:
    if stalled == %Stalled.visible:
        return
    %Stalled.visible = stalled
    if _pulse:
        _pulse.kill()
        _pulse = null
    if not stalled:
        return
    _pulse = create_tween().set_loops()
    _pulse.tween_property(%Stalled, "modulate:a", STALLED_PULSE_MIN_ALPHA, STALLED_PULSE_SEC * 0.5)
    _pulse.tween_property(%Stalled, "modulate:a", 1.0, STALLED_PULSE_SEC * 0.5)
