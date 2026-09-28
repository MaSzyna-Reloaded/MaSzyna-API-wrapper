extends PanelContainer

## A Lua editor for the scenario being played. Check compiles the code without running it; Apply
## runs it in the scenario's script context, where it has maszyna.* and eu07.events as a scenery
## script does (ScenarioScriptServer). Applying again first takes back what the previous Apply
## made - its events, timers and subscriptions - so the code can be changed and applied at will.

## The close button asks the owner of the View menu to hide the panel and untick its entry
signal close_requested

## The unit the editor's code runs as - the name its errors carry
const UNIT:StringName = &"editor"
const STATUS_OK_COLOR:Color = Color(0.55, 0.85, 0.6)
const STATUS_ERROR_COLOR:Color = Color(1.0, 0.55, 0.5)

var _context:RID = RID()


func _ready() -> void:
    ScenarioScriptServer.script_error.connect(_on_script_error)


func _exit_tree() -> void:
    ScenarioScriptServer.script_error.disconnect(_on_script_error)


## The script context of the scenario being played; an invalid RID while none is
func attach_context(context:RID) -> void:
    _context = context
    %CheckButton.disabled = not context.is_valid()
    %ApplyButton.disabled = not context.is_valid()
    %Status.text = ""


func _on_check_button_pressed() -> void:
    var error:String = ScenarioScriptServer.context_check_source(_context, UNIT, %Code.text)
    if error:
        _show_status(error, STATUS_ERROR_COLOR)
        return
    _show_status(tr("No errors"), STATUS_OK_COLOR)


## A failed run is reported by script_error, as the errors of what it left running are later
func _on_apply_button_pressed() -> void:
    if ScenarioScriptServer.context_apply_source(_context, UNIT, %Code.text):
        _show_status(tr("Applied"), STATUS_OK_COLOR)


func _on_script_error(context:RID, message:String) -> void:
    if context == _context:
        _show_status(message, STATUS_ERROR_COLOR)


func _show_status(text:String, color:Color) -> void:
    %Status.text = text
    %Status.add_theme_color_override(&"font_color", color)


func _on_close_button_pressed() -> void:
    close_requested.emit()
