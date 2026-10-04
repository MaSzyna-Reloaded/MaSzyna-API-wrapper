class_name SettingSlider
extends SettingRow

## A number in a range: a slider, the value with its unit beside it and a button - its icon alone,
## its name a tooltip - that resets it to its default. The value is a field the player can type it
## into - Enter or leaving the field stores exactly what was typed, kept only within the range
## (unless it is open that way); the slider shows it as near as its steps go, and what is no number
## puts the stored value back. An advanced row stands on one line: the name, the slider, the value.
## hint_string is a range hint's - "min,max,step", optionally followed by "or_greater", "or_less"
## and "suffix:<unit>".

const SUFFIX_PREFIX: String = "suffix:"
const OR_GREATER: String = "or_greater"
const OR_LESS: String = "or_less"
## The unit beside an advanced slider's value, smaller as its name is
const ADVANCED_UNIT_FONT_SIZE: int = 12
## An advanced row's line shared between the name and the slider in this proportion - so every
## slider of a window has one width and they stand in a column whatever the names, in the wide
## settings window as in the narrow panel beside the game (the value field's and the unit's columns
## are the scene's, the same for every row)
const ADVANCED_NAME_SHARE: float = 2.0
const ADVANCED_SLIDER_SHARE: float = 3.0

var _step: float = 0.0
var _suffix: String = ""


func _setup() -> void:
    var parts: PackedStringArray = hint_string.split(",")
    _step = float(parts[2])
    for part: String in parts.slice(3):
        if part.begins_with(SUFFIX_PREFIX):
            _suffix = part.trim_prefix(SUFFIX_PREFIX)
    %Slider.allow_greater = parts.has(OR_GREATER)
    %Slider.allow_lesser = parts.has(OR_LESS)
    %Slider.min_value = float(parts[0])
    %Slider.max_value = float(parts[1])
    %Slider.step = _step
    # the unit's column stays when there is no unit, so the fields stand in line
    %Unit.text = _suffix


## On one line with its name, all of it smaller
func _make_compact() -> void:
    var name_label: Label = %Title
    name_label.reparent(%Line)
    # moved, it is the scene's unique name again
    name_label.owner = self
    name_label.unique_name_in_owner = true
    %Line.move_child(name_label, 0)
    name_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    name_label.size_flags_stretch_ratio = ADVANCED_NAME_SHARE
    %Slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    %Slider.size_flags_stretch_ratio = ADVANCED_SLIDER_SHARE
    %Slider.set_compact(true)
    %ValueField.set_compact(true)
    %Unit.add_theme_font_size_override(&"font_size", ADVANCED_UNIT_FONT_SIZE)


func _enable_control(enabled: bool) -> void:
    %Slider.editable = enabled
    %Slider.modulate.a = 1.0 if enabled else UNAVAILABLE_ALPHA
    %ValueField.editable = enabled
    %ValueField.modulate.a = 1.0 if enabled else UNAVAILABLE_ALPHA
    _update_reset_button()


func _show(value: Variant) -> void:
    %Slider.value = float(value)
    # the stored value, which a typed one may hold finer than the slider's steps
    _show_value(float(value))


func _step_up() -> void:
    %Slider.value += _step


func _step_down() -> void:
    %Slider.value -= _step


## Stored first: what is shown - the reset among it - reads the stored value
func _on_slider_value_changed(value: float) -> void:
    _store(value)
    _show_value(value)


## As many decimals as the steps have - more when a typed value holds them
func _show_value(value: float) -> void:
    if is_equal_approx(value, snappedf(value, _step)):
        %ValueField.text = String.num(value, step_decimals(_step))
    else:
        %ValueField.text = String.num(value)
    _update_reset_button()


## The reset takes a value back to the default - there is nothing to take back at the default, nor
## on a row that is not available
func _update_reset_button() -> void:
    %ResetButton.disabled = not available or is_equal_approx(float(_read()), float(default_value))


## The field takes the keys to type the value, or gives them back to the slider (and takes what was
## typed, leaving)
func toggle_value_typing() -> void:
    if %ValueField.has_focus():
        %ValueField.release_focus()
    else:
        %ValueField.grab_focus()


## Enter gives the keyboard back to the page, and leaving the field takes what was typed
func _on_value_field_text_submitted(_text: String) -> void:
    %ValueField.release_focus()


## What was typed is stored as it is - the slider, which would round it to its step, only shows it
func _on_value_field_focus_exited() -> void:
    if not %ValueField.text.is_valid_float():
        _show_value(float(_read()))
        return
    var typed: float = float(%ValueField.text)
    if not %Slider.allow_greater:
        typed = minf(typed, %Slider.max_value)
    if not %Slider.allow_lesser:
        typed = maxf(typed, %Slider.min_value)
    %Slider.set_value_no_signal(typed)
    _store(typed)
    _show_value(typed)
