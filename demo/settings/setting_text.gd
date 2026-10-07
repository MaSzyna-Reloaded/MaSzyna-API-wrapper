class_name SettingText
extends SettingRow

## A setting that is a text the player types: a field. Enter on the row gives the field the keys,
## Enter in the field gives them back to the page; what was typed is stored as the field is left.


func _make_compact() -> void:
    %Field.set_compact(true)


func _enable_control(enabled: bool) -> void:
    %Field.editable = enabled
    %Field.modulate.a = 1.0 if enabled else UNAVAILABLE_ALPHA


func _show(value: Variant) -> void:
    %Field.text = str(value)


## Enter does not step a text, it types it
func is_edited_by_steps() -> bool:
    return false


func _activate() -> void:
    %Field.grab_focus()


func _on_field_text_submitted(_text: String) -> void:
    %Field.release_focus()


func _on_field_focus_exited() -> void:
    _store(%Field.text.strip_edges())
