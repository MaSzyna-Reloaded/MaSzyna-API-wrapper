extends Control

## The simulation's speed at hand: slowed, paused, or the wall clock's times one to thirty. What it
## shows is SimulationServer's - the speed and the pause change there, from this panel, the weather
## window or a script alike, and the panel lights the button of what is set.

## The speeds offered and their buttons' texts: slowed, then the wall clock's and its multiples
const SPEEDS:Array[float] = [0.1, 0.5, 1.0, 2.0, 4.0, 8.0, 30.0]
const SPEED_TEXTS:Array[String] = ["0.1", "0.5", "▶", "▶▶", "▶▶▶", "▶▶▶▶", "▶▶▶▶▶"]
## The pause button stands after the slowed speeds
const PAUSE_INDEX:int = 2

var _buttons:ButtonGroup = ButtonGroup.new()
var _speed_buttons:Array[Button] = []


func _ready() -> void:
    DrivingAid.apply_style(%Tile)
    %PauseButton.button_group = _buttons
    for index:int in SPEEDS.size():
        var button:Button = Button.new()
        button.text = SPEED_TEXTS[index]
        button.tooltip_text = tr("Simulation speed: x%s") % String.num(SPEEDS[index])
        button.flat = true
        button.toggle_mode = true
        button.focus_mode = Control.FOCUS_NONE
        button.theme_type_variation = &"SimulationSpeedButton"
        button.button_group = _buttons
        button.pressed.connect(_on_speed_button_pressed.bind(SPEEDS[index]))
        %Buttons.add_child(button)
        _speed_buttons.append(button)
    %Buttons.move_child(%PauseButton, PAUSE_INDEX)
    SimulationServer.simulation_speed_changed.connect(_show_speed)
    SimulationServer.simulation_paused.connect(_show_speed)
    SimulationServer.simulation_unpaused.connect(_show_speed)
    _show_speed()


func _exit_tree() -> void:
    SimulationServer.simulation_speed_changed.disconnect(_show_speed)
    SimulationServer.simulation_paused.disconnect(_show_speed)
    SimulationServer.simulation_unpaused.disconnect(_show_speed)


## The speed goes on running: a paused simulation starts at it
func _on_speed_button_pressed(speed:float) -> void:
    SimulationServer.simulation_speed = speed
    SimulationServer.simulation_unpause()


func _on_pause_button_pressed() -> void:
    SimulationServer.simulation_pause()


## The pause's button while paused, else the speed's; none for a speed no button offers (the
## weather window's slider has more)
func _show_speed() -> void:
    var index:int = SPEEDS.find(SimulationServer.simulation_speed)
    var lit:Button = %PauseButton if SimulationServer.simulation_is_paused() else (_speed_buttons[index] if index >= 0 else null)
    %PauseButton.set_pressed_no_signal(lit == %PauseButton)
    for button:Button in _speed_buttons:
        button.set_pressed_no_signal(button == lit)
