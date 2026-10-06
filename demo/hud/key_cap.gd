class_name KeyCap
extends PanelContainer

## A key drawn as a key: the name of an input event on a cap. Its look is the `KeyCap` and
## `KeyCapLabel` theme types of whoever shows it (the help's, the hints').


## Shows `event` as InputEventNames names it
func show_event(event:InputEvent) -> void:
    %Text.text = InputEventNames.event_name(event)
