@tool
extends EditorInspectorPlugin
class_name SfxAutomationInspectorPlugin

const BUTTON_TEXT := "Auto Sync Phase + Loops"


func _can_handle(object: Object) -> bool:
    return object is SfxAutomation


func _parse_begin(object: Object) -> void:
    var automation := object as SfxAutomation
    if automation == null:
        return

    var container := VBoxContainer.new()
    container.size_flags_horizontal = Control.SIZE_EXPAND_FILL

    var button := Button.new()
    button.text = BUTTON_TEXT
    button.tooltip_text = "Analyze track timing, set phase offsets, and rewrite supported loop offsets."
    button.disabled = _count_stream_tracks(automation) < 2
    container.add_child(button)

    var confirmation := ConfirmationDialog.new()
    confirmation.title = BUTTON_TEXT
    confirmation.dialog_text = "Analyze the current automation and apply phase/loop changes?"
    confirmation.ok_button_text = "Analyze"
    container.add_child(confirmation)

    var status := Label.new()
    status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    status.text = "Requires at least two tracks with AudioStream resources."
    if not button.disabled:
        status.text = "Analyzes PCM from the current automation and writes phase + loop settings."
    container.add_child(status)

    button.pressed.connect(_on_sync_confirmation_requested.bind(confirmation))
    confirmation.confirmed.connect(_on_sync_pressed.bind(automation, button, status))
    add_custom_control(container)


func _on_sync_confirmation_requested(confirmation: ConfirmationDialog) -> void:
    if confirmation == null:
        return
    confirmation.popup_centered()


func _on_sync_pressed(automation: SfxAutomation, button: Button, status: Label) -> void:
    button.disabled = true
    status.text = "Analyzing tracks..."

    var result: Dictionary = SfxAutomationSyncAnalyzer.analyze_automation(automation)
    if not result.get("ok", false):
        status.text = result.get("error", "Auto-sync failed.")
        button.disabled = false
        return

    var touched_stream_paths := PackedStringArray()
    var warnings: PackedStringArray = result.get("warnings", PackedStringArray())

    automation.phase_locked = true
    automation.phase_period = result["phase_period"]

    for track_result in result["track_results"]:
        var track: SfxTrack = track_result["track"]
        track.phase_offset = track_result["phase_offset"]

        if track_result.has("loop_offset"):
            var loop_apply := _apply_loop_offset(track, track_result["loop_offset"])
            if loop_apply.get("ok", false):
                var stream_path: String = loop_apply.get("stream_path", "")
                if not stream_path.is_empty() and not touched_stream_paths.has(stream_path):
                    touched_stream_paths.append(stream_path)
            else:
                var error_text: String = loop_apply.get("error", "")
                if not error_text.is_empty():
                    warnings.append(error_text)

    automation.emit_changed()
    var save_result := _save_automation_owner(automation)
    if not save_result.get("ok", false):
        var save_error: String = save_result.get("error", "")
        if not save_error.is_empty():
            warnings.append(save_error)

    _reimport_streams(touched_stream_paths)

    var summary := "Applied phase sync to %d tracks. phase_period=%.4fs" % [
        result["track_results"].size(),
        result["phase_period"],
    ]
    if not warnings.is_empty():
        summary += "\nWarnings: %s" % [", ".join(warnings)]
    status.text = summary
    button.disabled = false


func _count_stream_tracks(automation: SfxAutomation) -> int:
    var count := 0
    for track in automation.tracks:
        if track != null and track.stream != null:
            count += 1
    return count


func _apply_loop_offset(track: SfxTrack, loop_offset: float) -> Dictionary:
    if track == null or track.stream == null:
        return {"ok": false, "error": "Track is missing an AudioStream."}
    if not track.stream.has_method("set_loop") or not track.stream.has_method("set_loop_offset"):
        return {"ok": false, "error": "Stream type does not support loop_offset editing."}

    var stream: AudioStream = track.stream
    stream.call("set_loop", true)
    stream.call("set_loop_offset", loop_offset)

    var stream_path := stream.resource_path
    if stream_path.is_empty():
        return {"ok": false, "error": "Stream has no resource_path, so loop_offset could not be persisted."}

    var import_path := "%s.import" % stream_path
    if not FileAccess.file_exists(import_path):
        return {"ok": false, "error": "Missing import config for %s." % stream_path}

    var config := ConfigFile.new()
    var load_error := config.load(import_path)
    if load_error != OK:
        return {"ok": false, "error": "Could not load %s." % import_path}

    config.set_value("params", "loop", true)
    config.set_value("params", "loop_offset", loop_offset)

    var save_error := config.save(import_path)
    if save_error != OK:
        return {"ok": false, "error": "Could not save %s." % import_path}

    return {
        "ok": true,
        "stream_path": stream_path,
    }


func _save_automation_owner(automation: SfxAutomation) -> Dictionary:
    var owner_path := _resolve_owner_path(automation.resource_path)
    if owner_path.is_empty():
        return {"ok": false, "error": "Automation resource has no save path; save it manually after apply."}
    if not owner_path.ends_with(".tres") and not owner_path.ends_with(".res"):
        return {"ok": false, "error": "Auto-save currently supports .tres/.res owners only; save the resource manually."}

    var owner_resource := ResourceLoader.load(owner_path, "", ResourceLoader.CACHE_MODE_REUSE)
    if owner_resource == null:
        return {"ok": false, "error": "Could not reload %s for saving." % owner_path}

    var save_error := ResourceSaver.save(owner_resource, owner_path)
    if save_error != OK:
        return {"ok": false, "error": "Could not save %s." % owner_path}
    return {"ok": true}


func _reimport_streams(stream_paths: PackedStringArray) -> void:
    if stream_paths.is_empty():
        return

    var filesystem := EditorInterface.get_resource_filesystem()
    if filesystem == null:
        return
    if filesystem.has_method("reimport_files"):
        filesystem.call("reimport_files", stream_paths)
    elif filesystem.has_method("scan"):
        filesystem.call_deferred("scan")


func _resolve_owner_path(resource_path: String) -> String:
    if resource_path.is_empty():
        return ""
    var separator := resource_path.find("::")
    if separator == -1:
        return resource_path
    return resource_path.substr(0, separator)
