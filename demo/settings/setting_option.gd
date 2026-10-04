class_name SettingOption
extends SettingRow

## A setting that is one of a few named values: a drop-down, and left and right walk it. hint_string
## is an enum hint's - "Name:value,Name:value", the names as msgids.


func _setup() -> void:
    for choice: String in hint_string.split(","):
        %Options.add_item(choice.get_slice(":", 0), int(choice.get_slice(":", 1)))


func _make_compact() -> void:
    %Options.set_compact(true)


func _enable_control(enabled: bool) -> void:
    %Options.disabled = not enabled
    %Options.modulate.a = 1.0 if enabled else UNAVAILABLE_ALPHA


func _show(value: Variant) -> void:
    %Options.select(%Options.get_item_index(_id_of(value)))


func _step_up() -> void:
    _choose(mini(%Options.selected + 1, %Options.item_count - 1))


func _step_down() -> void:
    _choose(maxi(%Options.selected - 1, 0))


func _activate() -> void:
    _choose(wrapi(%Options.selected + 1, 0, %Options.item_count))


## select() emits nothing, so a key stores here what a click stores through item_selected
func _choose(index: int) -> void:
    if index == %Options.selected:
        return
    %Options.select(index)
    _store(_value_of(%Options.get_item_id(index)))


func _on_options_item_selected(index: int) -> void:
    _store(_value_of(%Options.get_item_id(index)))


## The stored value of a choice - the choice's own id here, a subclass's value of it
func _value_of(id: int) -> Variant:
    return id


func _id_of(value: Variant) -> int:
    return int(value)
