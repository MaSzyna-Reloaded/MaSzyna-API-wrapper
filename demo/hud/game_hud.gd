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
    $ControlWindows/TrackAndTraction/TrackTractionPanel.player_path = NodePath(
        "../../../%s" % player_path)


func _ready() -> void:
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    if player:
        player.controlled_vehicle_changed.connect(_bind_vehicle.bind(null))
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
    _bind_vehicle(win)


func _on_show_all_controls_button_toggled(toggled_on: bool) -> void:
    for win: Node in $ControlWindows.get_children():
        win.visible = toggled_on
        _bind_vehicle(win)


## The vehicle the player is driving, handed to every widget that shows something about it. The
## widgets used to be given a NodePath into the vehicle's own subtree and resolve it themselves,
## which reached across two scenes and could resolve before the vehicle had been built. They are
## given the vehicle itself now, and the player says when it changes.
func _bind_vehicle(node: Node = null) -> void:
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    var vehicle: RailVehicle3D = player.controlled_vehicle if player else null
    var controller: VehicleController = vehicle.get_controller() if vehicle else null
    _propagate_vehicle(node if node else $ControlWindows, controller)


func _propagate_vehicle(node: Node, controller: VehicleController) -> void:
    for child: Node in node.get_children():
        if "vehicle" in child:
            child.vehicle = controller
        _propagate_vehicle(child, controller)
