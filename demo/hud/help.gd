extends ScrollContainer
class_name Help

## Lists input actions defined in project settings as "Action [Shortcut]" rows, grouped by GROUPS.
## Actions not listed in any group go to DEFAULT_GROUP; built-in ui_* actions are skipped.

const DEFAULT_GROUP: String = "Vehicle"
const GROUPS: Dictionary = {
    "Vehicle": [],
    "Player": [
        "change_vehicle",
        "cabin_mode_toggle",
        "cabin_next",
        "cabin_previous",
        "external_view_cycle",
        "flashlight_toggle",
        "coupler_connect",
        "coupler_disconnect",
        "coupler_disconnect_occupied",
    ],
    "UI": [
        "hud_toggle",
        "toggle_weather_controls",
    ],
}


func _ready() -> void:
    var grouped_actions: Dictionary = {}
    for group: String in GROUPS:
        grouped_actions[group] = PackedStringArray()

    for property: Dictionary in ProjectSettings.get_property_list():
        var property_name: String = property["name"]
        if not property_name.begins_with("input/") or property_name.begins_with("input/ui_"):
            continue
        var action: String = property_name.trim_prefix("input/")
        var action_group: String = DEFAULT_GROUP
        for group: String in GROUPS:
            if action in GROUPS[group]:
                action_group = group
        grouped_actions[action_group].append(action)

    for group: String in GROUPS:
        var actions: PackedStringArray = grouped_actions[group]
        if not actions:
            continue
        actions.sort()
        var foldable: FoldableContainer = FoldableContainer.new()
        foldable.title = group
        var rows: VBoxContainer = VBoxContainer.new()
        for action: String in actions:
            rows.add_child(_make_row(action))
        foldable.add_child(rows)
        %Bindings.add_child(foldable)


func _make_row(action: String) -> HBoxContainer:
    var row: HBoxContainer = HBoxContainer.new()
    var label: Label = Label.new()
    label.text = action.capitalize()
    label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    label.custom_minimum_size.x = 80.0
    label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    row.add_child(label)
    for event: InputEvent in ProjectSettings.get_setting("input/" + action)["events"]:
        var key_cap: PanelContainer = PanelContainer.new()
        key_cap.theme_type_variation = &"KeyCap"
        key_cap.size_flags_vertical = Control.SIZE_SHRINK_CENTER
        var key_label: Label = Label.new()
        key_label.theme_type_variation = &"KeyCapLabel"
        key_label.text = _format_event(event)
        key_cap.add_child(key_label)
        row.add_child(key_cap)
    return row


func _format_event(event: InputEvent) -> String:
    if event is InputEventKey:
        var key: InputEventKey = event as InputEventKey
        if key.keycode:
            return key.as_text_keycode()
        return key.as_text_physical_keycode()
    return event.as_text()
