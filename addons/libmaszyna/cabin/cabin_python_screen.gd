extends Node
class_name CabinPythonScreen

## A cab screen drawn by a Python 2 script (`pyscreen:` of an MMD). The texture PythonScreenServer
## draws replaces the one of the target submodel, and the screen is redrawn from the vehicle's
## state at its own interval (TTrain::update_screens(), Train.cpp:10288).

## The albedo texture of the cab's materials (MaterialFactory)
const TEXTURE_PARAMETER:StringName = &"texture_albedo"
const MSEC_PER_SEC:float = 1000.0

var vehicle_rid:RID
## Absolute path of the script, without ".py"
var script_path:String = ""
## The screen's `parameters:` from the MMD
var parameters:Dictionary = {}
## Redraw interval in milliseconds; -1 draws the screen once
var update_time_msec:int = 0
## Submodel the screen is shown on; null for target "none", whose script still runs
var mesh:MeshInstance3D = null

var _screen:RID = RID()
## Script commands with no wrapper equivalent yet, reported once each
var _unsupported_commands:Dictionary[String, bool] = {}


func _enter_tree() -> void:
    _screen = PythonScreenServer.screen_create(script_path, _on_commands_received)
    if mesh:
        # the cab's materials are shared through MaterialManager's cache - the screen gets its own
        var material:ShaderMaterial = mesh.material_override.duplicate()
        material.set_shader_parameter(TEXTURE_PARAMETER, PythonScreenServer.screen_get_texture(_screen))
        mesh.material_override = material


func _exit_tree() -> void:
    PythonScreenServer.screen_free(_screen)


func _ready() -> void:
    if update_time_msec < 0:
        _render()
        return
    var timer := Timer.new()
    timer.wait_time = update_time_msec / MSEC_PER_SEC
    timer.autostart = true
    timer.timeout.connect(_render)
    add_child(timer)


func _render() -> void:
    PythonScreenServer.screen_request_render(_screen, PythonScreenState.compose(vehicle_rid, parameters))


## "command;param1;param2" (PyInt.cpp:151) - the original's command names are not mapped to the
## wrapper's commands yet (TODO.md)
func _on_commands_received(commands:PackedStringArray) -> void:
    for command:String in commands:
        var command_name:String = command.get_slice(";", 0)
        if not command_name in _unsupported_commands:
            _unsupported_commands[command_name] = true
            push_warning("CabinPythonScreen: %s sends '%s', which is not supported" % [script_path, command_name])
