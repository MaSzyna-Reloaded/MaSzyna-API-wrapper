extends ScrollContainer
class_name Help

## Lists input actions defined in project settings as "Action [Shortcut]" rows, grouped by GROUPS.
## Actions not listed in any group go to DEFAULT_GROUP; built-in ui_* actions are skipped.

const KEY_CAP: PackedScene = preload("key_cap.tscn")
const DEFAULT_GROUP: String = "Vehicle"
const GROUPS: Dictionary = {
    "Vehicle": [],
    "Player": [
        "change_vehicle",
        "cabin_mode_toggle",
        "cabin_sit_down",
        "cabin_next",
        "cabin_previous",
        "external_view_cycle",
        "ai_driver_enable",
        "ai_driver_disable",
        "flashlight_toggle",
        "coupler_connect",
        "coupler_disconnect",
        "coupler_disconnect_occupied",
        "coupler_adapter_attach",
        "coupler_adapter_remove",
    ],
    "UI": [
        "hud_toggle",
        "minimap_toggle",
        "toggle_weather_controls",
        "timetable_toggle",
        "scenario_toggle",
        "hints_toggle",
        "trainsets_toggle",
        "vehicle_card_toggle",
        "cycle_debug_menu",
        "console_toggle",
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
        var foldable: UIFoldable = UIFoldable.new()
        foldable.title = group
        # Tab and Space belong to the game, not to the focus of a HUD widget
        foldable.focus_mode = Control.FOCUS_NONE
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
        var key_cap: KeyCap = KEY_CAP.instantiate()
        row.add_child(key_cap)
        key_cap.show_event(event)
    return row

