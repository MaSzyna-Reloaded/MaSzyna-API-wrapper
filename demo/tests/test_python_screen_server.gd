extends MaszynaGutTest

## PythonScreenServer runs a Python 2 cab screen script and turns what it draws into a texture.
## The pixels themselves cannot be read back headless (the dummy renderer keeps no texture data),
## so the fixture sends the state it received back as a command.
## Needs a CPython 2.7 runtime: MASZYNA_PYTHON_HOME names its prefix (the directory holding
## lib/libpython2.7.so.1.0); without it the test is skipped.

const SCRIPT:String = "res://tests/fixtures/python/fixture_screen"
const HOME_SETTING:String = "maszyna/python/home"
const RENDER_TIMEOUT_SEC:float = 10.0

var _previous_home:String = ""
var _screen:RID = RID()
var _commands:PackedStringArray = PackedStringArray()


func before_each() -> void:
    _previous_home = ProjectSettings.get_setting(HOME_SETTING, "")


func after_each() -> void:
    PythonScreenServer.screen_free(_screen)
    ProjectSettings.set_setting(HOME_SETTING, _previous_home)


func test_script_draws_the_state_into_the_texture_and_sends_commands() -> void:
    var home:String = OS.get_environment("MASZYNA_PYTHON_HOME")
    if not home:
        pending("MASZYNA_PYTHON_HOME is not set")
        return
    ProjectSettings.set_setting(HOME_SETTING, home)
    _screen = PythonScreenServer.screen_create(ProjectSettings.globalize_path(SCRIPT), _on_commands_received)

    var touches:Array = [Vector2(0.25, 0.5)]
    var state:Dictionary = {"level": 1.0, "step": 128, "enabled": true, "name": "ep07-424", "touches": touches}
    PythonScreenServer.screen_request_render(_screen, state)
    await wait_until(func() -> bool: return _commands.size() > 0, RENDER_TIMEOUT_SEC)

    assert_eq(_commands, PackedStringArray([
        "RGBA", ProjectSettings.globalize_path(SCRIPT).get_base_dir() + "/",
        "1.0;128;True;ep07-424;[(0.25, 0.5)]"]))
    var texture:Texture2D = PythonScreenServer.screen_get_texture(_screen)
    assert_eq(texture.get_size(), Vector2(2, 1))


func _on_commands_received(commands:PackedStringArray) -> void:
    _commands = commands
