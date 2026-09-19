extends Node3D

## Menu order snapshot - HUDWindow.move_to_front() reorders ControlWindows children.
var _windows: Array[HUDWindow] = []


func _ready() -> void:
    var menu: PopupMenu = $TopBar/HBoxContainer/MenuBar/PopupMenu as PopupMenu
    for child: Node in $ControlWindows.get_children():
        var win: HUDWindow = child as HUDWindow
        win.visible = false
        menu.add_item(win.title)
        _windows.append(win)
    $ControlWindows/Scenery.visible = not $MaszynaSceneryNode.filename


func _input(event: InputEvent) -> void:
    if event.is_action_pressed("hud_toggle"):
        $TopBar/HBoxContainer/ToggleAllControls.button_pressed = not $TopBar/HBoxContainer/ToggleAllControls.button_pressed
    if event.is_action_pressed("toggle_weather_controls"):
        $ControlWindows/WeatherAndTime.visible = not $ControlWindows/WeatherAndTime.visible


func _on_popup_menu_index_pressed(index: int) -> void:
    var win: HUDWindow = _windows[index]
    win.visible = not win.visible
    _bind_train_controller(win)


func _on_show_all_controls_button_toggled(toggled_on: bool) -> void:
    for win: Node in $ControlWindows.get_children():
        win.visible = toggled_on
        _bind_train_controller(win)


func _on_scenery_selector_scenery_selected(filename: String) -> void:
    $ControlWindows/Scenery.visible = false
    $TopBar.visible = false
    $ControlWindows.visible = false
    $LoadingScreen.show_loading(filename.get_basename())
    $MaszynaSceneryNode.filename = filename
    await $Player.clear_start_train()
    await $MaszynaSceneryNode.load()
    $LoadingScreen.visible = false
    $TopBar.visible = true
    $ControlWindows.visible = true


func _bind_train_controller(win: Node) -> void:
    if not $Player.controlled_vehicle:
        return
    for child: Node in win.get_children():
        if "train_controller" in child:
            child.train_controller = child.get_path_to($Player.controlled_vehicle.get_controller())
