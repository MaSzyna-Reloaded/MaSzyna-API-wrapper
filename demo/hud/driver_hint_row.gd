class_name DriverHintRow
extends HBoxContainer

## One hint of the hints chip: the key that does it on its left, when one does, and its text.


## `event` is the key's (null for none), `done` shows the step the vehicle already shows done
func show_hint(event:InputEvent, text:String, done:bool) -> void:
    %KeyCap.visible = event != null
    if event:
        %KeyCap.show_event(event)
    %Text.text = text
    %Text.theme_type_variation = &"DrivingAidHintDone" if done else &"DrivingAidHint"
