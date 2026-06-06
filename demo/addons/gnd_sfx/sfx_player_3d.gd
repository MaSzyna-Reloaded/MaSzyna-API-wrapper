@tool
extends Node3D
class_name SfxPlayer3D

const PLAYBACK_NONE_OPTION := "<none>"

signal finished

var _runtime = SfxPlaybackRuntime.new()

@export var bank: SfxBank:
    set(value):
        bank = value
        _events_changed()

@export var max_tracks: int = 10:
    set(value):
        max_tracks = value
        sync_values(true)

@export var attenuation_model: AudioStreamPlayer3D.AttenuationModel = AudioStreamPlayer3D.ATTENUATION_INVERSE_DISTANCE:
    set(value):
        attenuation_model = value
        sync_values()

@export_range(0.01, 100.0, 0.01) var unit_size: float = 10:
    set(value):
        unit_size = value
        sync_values()

@export var max_distance: int = 0:
    set(value):
        max_distance = value
        sync_values()

@export var max_polyphony: int = 1:
    set(value):
        max_polyphony = value
        sync_values()

@export_range(0.0, 3.0) var panning_strength: float = 1.0:
    set(value):
        panning_strength = value
        sync_values()

@export_group("Playback", "playback")
@export var playback_enabled: bool = false:
    set(value):
        playback_enabled = value
        sync_values()

@export var playback_effect: StringName = "":
    set(value):
        if value == PLAYBACK_NONE_OPTION:
            value = &""
        playback_effect = value
        _sanitize_playback_selection()
        _notify_playback_property_list_changed()
        sync_values()

@export var playback_automation: StringName = "":
    set(value):
        if value == PLAYBACK_NONE_OPTION:
            value = &""
        playback_automation = value
        sync_values()

@export var playback_automation_value: float = 0.0:
    set(value):
        playback_automation_value = value
        sync_values()

var _preview_enabled: bool = false
var _preview_effect: StringName = &""
var _preview_automation: StringName = &""
var _preview_automation_value: float = 0.0
var _watched_events: Array[SfxEvent] = []
var _watched_automations: Array[SfxAutomation] = []
var _players: Array = []


func _enter_tree() -> void:
    _reset_playback_preview_state()
    if Engine.is_editor_hint():
        playback_enabled = false
        _notify_playback_property_list_changed()


func _ready() -> void:
    _ensure_runtime_connections()
    _connect_playback_resource_watchers()
    sync_values(true)
    _update_process_state()


func _events_changed() -> void:
    _disconnect_playback_resource_watchers()
    _connect_playback_resource_watchers()
    _sanitize_playback_selection()
    _notify_playback_property_list_changed()


func _exit_tree() -> void:
    _disconnect_playback_resource_watchers()


func _process(delta: float) -> void:
    _runtime.update(delta)


func sync_values(rebuild := false) -> void:
    _ensure_runtime_connections()
    if rebuild:
        _runtime.clear()
        for player in _players:
            if is_instance_valid(player):
                remove_child(player)
                player.queue_free()
        _players = []
        var index := 0
        while index < max_tracks:
            var player := AudioStreamPlayer3D.new()
            _players.append(player)
            add_child(player)
            player.finished.connect(_on_player_finished.bind(player))
            index += 1
        _runtime.set_players(_players)

    for player in _players:
        player.max_polyphony = max_polyphony
        player.max_distance = max_distance
        player.panning_strength = panning_strength
        player.attenuation_model = attenuation_model
        player.unit_size = unit_size

    _sync_editor_playback()
    _update_process_state()


func play(event_name: StringName, offset_or_parameters = null, parameters: Dictionary = {}) -> void:
    if not bank:
        return

    var offset := 0.0
    if offset_or_parameters is Dictionary:
        parameters = offset_or_parameters
    elif offset_or_parameters != null:
        offset = float(offset_or_parameters)

    var event := bank.get_event(event_name)
    if event != null:
        _runtime.play(event, offset, parameters)


func seek(event_name: StringName, offset: float) -> void:
    _runtime.seek(event_name, offset)


func modulate(event_name: StringName, parameters: Dictionary) -> void:
    _runtime.modulate(event_name, parameters)


func set_parameters(parameters: Dictionary) -> void:
    _runtime.set_parameters(parameters)


func stop(event_name_or_immediate = null, immediate: bool = false) -> void:
    _runtime.stop(event_name_or_immediate, immediate)


func is_playing(event_name: StringName) -> bool:
    return _runtime.is_playing(event_name)


func play_automation(event_name: StringName, automation_name: StringName, value: float = 0.0, restart: bool = false) -> void:
    if not bank:
        return

    var event := bank.get_event(event_name)
    if event != null:
        _runtime.play_automation(event, automation_name, value, restart)


func stop_automation(event_name: StringName, automation_name: StringName, immediate: bool = false) -> void:
    if not bank:
        return

    var event := bank.get_event(event_name)
    if event != null:
        _runtime.stop_automation(event, automation_name, immediate)


func _ensure_runtime_connections() -> void:
    if not _runtime.process_requirement_changed.is_connected(_on_runtime_process_requirement_changed):
        _runtime.process_requirement_changed.connect(_on_runtime_process_requirement_changed)
    if not _runtime.finished.is_connected(_on_runtime_finished):
        _runtime.finished.connect(_on_runtime_finished)


func _on_runtime_process_requirement_changed(required: bool) -> void:
    if Engine.is_editor_hint():
        _update_process_state()
        return
    set_process(required)


func _on_runtime_finished() -> void:
    finished.emit()


func _on_player_finished(player: AudioStreamPlayer3D) -> void:
    _runtime.handle_player_finished(player)


func _sync_editor_playback() -> void:
    if not Engine.is_editor_hint() or not is_inside_tree():
        return

    var config_changed := (
        playback_enabled != _preview_enabled
        or playback_effect != _preview_effect
        or playback_automation != _preview_automation
    )
    var value_changed := not is_equal_approx(playback_automation_value, _preview_automation_value)

    if not config_changed and not value_changed:
        return

    if not playback_enabled or String(playback_effect).is_empty():
        #_runtime.clear()
        if _preview_enabled and _preview_effect:
            stop(_preview_effect)
        else:
            _runtime.clear()
        _store_playback_preview_state()
        return

    if config_changed:
        _runtime.clear()
        play(playback_effect, 0.0, _build_preview_parameters())
        _store_playback_preview_state()
        return

    if value_changed and not String(playback_automation).is_empty():
        modulate(playback_effect, {playback_automation: playback_automation_value})
        _store_playback_preview_state()


func _build_preview_parameters() -> Dictionary:
    if String(playback_automation).is_empty():
        return {}
    return {playback_automation: playback_automation_value}


func _store_playback_preview_state() -> void:
    _preview_enabled = playback_enabled
    _preview_effect = playback_effect
    _preview_automation = playback_automation
    _preview_automation_value = playback_automation_value


func _reset_playback_preview_state() -> void:
    _preview_enabled = false
    _preview_effect = &""
    _preview_automation = &""
    _preview_automation_value = 0.0


func _update_process_state() -> void:
    if Engine.is_editor_hint():
        set_process(_runtime.requires_process())


func _validate_property(property: Dictionary) -> void:
    if property.name == "playback_effect":
        property.hint = PROPERTY_HINT_ENUM
        property.hint_string = _build_playback_effect_hint()
    elif property.name == "playback_automation":
        property.hint = PROPERTY_HINT_ENUM
        property.hint_string = _build_playback_automation_hint()


func _build_playback_effect_hint() -> String:
    if not bank:
        return ""

    var options := PackedStringArray([PLAYBACK_NONE_OPTION])
    for event in bank.events:
        if event == null or String(event.name).is_empty():
            continue
        options.append(String(event.name))
    return ",".join(options)


func _build_playback_automation_hint() -> String:
    var options := PackedStringArray([PLAYBACK_NONE_OPTION])
    var event := _find_playback_event()
    if event == null:
        return ",".join(options)

    for automation in event.automations:
        if automation == null or String(automation.parameter_name).is_empty():
            continue
        options.append(String(automation.parameter_name))
    return ",".join(options)


func _find_playback_event() -> SfxEvent:
    if not bank or not playback_effect:
        return null
    return bank.get_event(playback_effect)


func _sanitize_playback_selection() -> void:
    var event := _find_playback_event()
    if not String(playback_effect).is_empty() and event == null:
        playback_effect = &""
        playback_automation = &""
        return

    if String(playback_automation).is_empty() or event == null:
        return

    for automation in event.automations:
        if automation != null and automation.parameter_name == playback_automation:
            return
    playback_automation = &""


func _notify_playback_property_list_changed() -> void:
    if Engine.is_editor_hint():
        notify_property_list_changed()


func _connect_playback_resource_watchers() -> void:
    if not Engine.is_editor_hint() or not bank:
        return

    for event in bank.events:
        if event == null or _watched_events.has(event):
            continue
        event.changed.connect(_on_playback_source_changed)
        _watched_events.append(event)

        for automation in event.automations:
            if automation == null or _watched_automations.has(automation):
                continue
            automation.changed.connect(_on_playback_source_changed)
            _watched_automations.append(automation)


func _disconnect_playback_resource_watchers() -> void:
    for event in _watched_events:
        if is_instance_valid(event) and event.changed.is_connected(_on_playback_source_changed):
            event.changed.disconnect(_on_playback_source_changed)
    for automation in _watched_automations:
        if is_instance_valid(automation) and automation.changed.is_connected(_on_playback_source_changed):
            automation.changed.disconnect(_on_playback_source_changed)
    _watched_events.clear()
    _watched_automations.clear()


func _on_playback_source_changed() -> void:
    _sanitize_playback_selection()
    sync_values()
