extends Node3D

@onready var world = $WorldEnvironment


func _ready():
    var menu = $TopBar/HBoxContainer/MenuBar/PopupMenu as PopupMenu
    for child in $ControlWindows.get_children():
        var win = child as HUDWindow
        if win:
            win.visible = false
            menu.add_item(win.title)


func _input(event):
    if event.is_action_pressed("hud_toggle"):
        $TopBar/HBoxContainer/ToggleAllControls.button_pressed = not $TopBar/HBoxContainer/ToggleAllControls.button_pressed


func _on_popup_menu_index_pressed(index: int) -> void:
    var win = $ControlWindows.get_child(index) as HUDWindow
    if win:
        win.visible = not win.visible
        for child in win.get_children():
            if "train_controller" in child:
                if $Player.controlled_vehicle:
                    child.train_controller = child.get_path_to($Player.controlled_vehicle.get_controller())


func _on_show_all_controls_button_toggled(toggled_on: bool) -> void:
    for win in $ControlWindows.get_children():
        win.visible = toggled_on
        for child in win.get_children():
            if "train_controller" in child:
                if $Player.controlled_vehicle:
                    child.train_controller = child.get_path_to($Player.controlled_vehicle.get_controller())
