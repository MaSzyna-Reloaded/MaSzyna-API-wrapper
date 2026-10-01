@tool
class_name SceneryLoadMeasurement
extends RefCounted

## What a scenery load costs, stage by stage (MaszynaIncludeNode.LoadStage): how long the stage
## took, its longest frame - the main thread blocked, and nothing on the loading screen moves - and
## the peak of static memory. Started by SceneryInstancer.instantiate() for one load and printed
## when it ends, one line per stage:
## [SceneryLoad] FILES 41.2 s, longest frame 0.90 s, peak 6.10 GB

const STAGE_NAMES: PackedStringArray = ["FILES", "INFRASTRUCTURE", "TERRAIN", "OBJECTS", "VEHICLES"]
const USEC_PER_SEC: float = 1000000.0
const BYTES_PER_GB: float = 1024.0 * 1024.0 * 1024.0

var _root: MaszynaIncludeNode = null
var _stage: MaszynaIncludeNode.LoadStage = MaszynaIncludeNode.LoadStage.FILES
var _stage_started_usec: int = 0
var _frame_usec: int = 0
var _durations_usec: PackedInt64Array = []
var _longest_frames_usec: PackedInt64Array = []
var _peaks_bytes: PackedInt64Array = []


func _init(root: MaszynaIncludeNode) -> void:
    _root = root
    _durations_usec.resize(STAGE_NAMES.size())
    _longest_frames_usec.resize(STAGE_NAMES.size())
    _peaks_bytes.resize(STAGE_NAMES.size())
    _stage_started_usec = Time.get_ticks_usec()
    _frame_usec = _stage_started_usec
    _root.load_progress.connect(_on_load_progress)
    _root.get_tree().process_frame.connect(_on_process_frame)


## The load has ended: the last stage is closed and every stage printed
func finish() -> void:
    _root.load_progress.disconnect(_on_load_progress)
    _root.get_tree().process_frame.disconnect(_on_process_frame)
    _close_stage()
    for stage: int in STAGE_NAMES.size():
        if _durations_usec[stage] == 0:
            continue
        print("[SceneryLoad] %s %.1f s, longest frame %.2f s, peak %.2f GB" % [
            STAGE_NAMES[stage], _durations_usec[stage] / USEC_PER_SEC,
            _longest_frames_usec[stage] / USEC_PER_SEC, _peaks_bytes[stage] / BYTES_PER_GB,
        ])


func _on_load_progress(_progress: float, stage: MaszynaIncludeNode.LoadStage, _message: String) -> void:
    if stage == _stage:
        return
    _close_stage()
    _stage = stage


func _on_process_frame() -> void:
    var now: int = Time.get_ticks_usec()
    _longest_frames_usec[_stage] = maxi(_longest_frames_usec[_stage], now - _frame_usec)
    _frame_usec = now
    _peaks_bytes[_stage] = maxi(_peaks_bytes[_stage], int(Performance.get_monitor(Performance.MEMORY_STATIC)))


func _close_stage() -> void:
    var now: int = Time.get_ticks_usec()
    _durations_usec[_stage] += now - _stage_started_usec
    _stage_started_usec = now
