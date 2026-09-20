extends Control

## HUD shared by the demo scenes: the top bar with its "Controls" menu and the windows it opens.
## A scene that needs a menu entry of its own adds a Button to MenuActions (editable children),
## like demo_scenery_loading.tscn does with "Exit to menu": the button is never shown, its text
## becomes an entry at the end of the menu and picking the entry emits its pressed signal.

## The player whose vehicle the control windows drive
@export var player_path: NodePath
@export var environment_node_path: NodePath

## Menu order snapshot - HUDWindow.move_to_front() reorders ControlWindows children.
var _windows: Array[HUDWindow] = []
var _menu_actions: Array[Button] = []


func _enter_tree() -> void:
    # WeatherControls resolves its path in _ready(), before this node's own _ready(); it sits three
    # levels below this node, so the path given relative to this node is rebased by that much
    $ControlWindows/WeatherAndTime/WeatherControls.environment_node_path = NodePath(
        "../../../%s" % environment_node_path)


func _ready() -> void:
    var menu: PopupMenu = $TopBar/HBoxContainer/MenuBar/PopupMenu as PopupMenu
    for child: Node in $ControlWindows.get_children():
        var win: HUDWindow = child as HUDWindow
        win.visible = false
        menu.add_item(win.title)
        _windows.append(win)
    for child: Node in $MenuActions.get_children():
        var action: Button = child as Button
        menu.add_item(action.text)
        _menu_actions.append(action)


func _input(event: InputEvent) -> void:
    if event.is_action_pressed("hud_toggle"):
        $TopBar/HBoxContainer/ToggleAllControls.button_pressed = not $TopBar/HBoxContainer/ToggleAllControls.button_pressed
    if event.is_action_pressed("toggle_weather_controls"):
        $ControlWindows/WeatherAndTime.visible = not $ControlWindows/WeatherAndTime.visible


func _on_popup_menu_index_pressed(index: int) -> void:
    if index >= _windows.size():
        _menu_actions[index - _windows.size()].pressed.emit()
        return
    var win: HUDWindow = _windows[index]
    win.visible = not win.visible
    _bind_train_controller(win)


func _on_show_all_controls_button_toggled(toggled_on: bool) -> void:
    for win: Node in $ControlWindows.get_children():
        win.visible = toggled_on
        _bind_train_controller(win)


func _bind_train_controller(win: Node) -> void:
    var player: MaszynaPlayer = get_node(player_path) as MaszynaPlayer
    if not player.controlled_vehicle:
        return
    for child: Node in win.get_children():
        if "train_controller" in child:
            child.train_controller = child.get_path_to(player.controlled_vehicle.get_controller())
