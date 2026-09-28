extends Control

## HUD shared by the demo scenes: the top bar with its "Controls" menu and the windows it opens.
## A scene that needs a menu entry of its own adds a Button to MenuActions (editable children),
## like demo_scenery_loading.tscn does with "Exit to menu": the button is never shown, its text
## becomes an entry at the end of the menu and picking the entry emits its pressed signal.

## The entries of the "View" menu
enum ViewItem { TRANSCRIPTS, DRIVING_AID, TIMETABLE, SCENARIO, CONTROLS, SCRIPTS, TRAINSETS }

const VEHICLE_CARD:PackedScene = preload("vehicle_card.tscn")
const FRONT_END:int = 0

## The player whose vehicle the control windows drive
@export var player_path: NodePath
@export var environment_node_path: NodePath

## Menu order snapshot - HUDWindow.move_to_front() reorders ControlWindows children.
var _windows: Array[HUDWindow] = []
var _menu_actions: Array[Button] = []
## The vehicle card of the selector's active row; a click on another row shows that row's vehicle
## in it; null while no row is active
var _card: VehicleCard = null
## Where the card was when it was last closed; it opens there again (no area until then)
var _card_rect: Rect2 = Rect2()


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
        player.controlled_vehicle_changed.connect(_on_controlled_vehicle_changed.bind(player))
        player.external_view_changed.connect(_on_player_external_view_changed)
        player.cabin_view_changed.connect(_on_player_cabin_view_changed.bind(player))
    RailVehicleServer.vehicle_freed.connect(_on_vehicle_freed)
    SceneryHUDMouseServer.vehicle_pressed.connect(_open_card)
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
    # the menus show their keys and handle them: a key picks its entry as a click would
    menu.set_item_shortcut(_windows.find($ControlWindows/WeatherAndTime), _action_shortcut(&"toggle_weather_controls"))
    # Tab, as the original's map panel (driveruilayer.cpp:136)
    menu.set_item_shortcut(_windows.find($ControlWindows/MiniMap), _action_shortcut(&"minimap_toggle"))
    # F2, as the original's (driveruilayer.cpp:155)
    %View.set_item_shortcut(ViewItem.TIMETABLE, _action_shortcut(&"timetable_toggle"))
    # Shift+F2 - free in the original, whose F-keys ignore modifiers (driveruilayer.cpp:115)
    %View.set_item_shortcut(ViewItem.SCENARIO, _action_shortcut(&"scenario_toggle"))
    # F12 - free in the original without Shift (driveruilayer.cpp:174)
    %View.set_item_shortcut(ViewItem.CONTROLS, _action_shortcut(&"hud_toggle"))
    # F1, as the original's driving aid (driveruilayer.cpp:76)
    %View.set_item_shortcut(ViewItem.DRIVING_AID, _action_shortcut(&"driving_aid_toggle"))


func _exit_tree() -> void:
    RailVehicleServer.vehicle_freed.disconnect(_on_vehicle_freed)
    SceneryHUDMouseServer.vehicle_pressed.disconnect(_open_card)


## A menu shortcut of an input action - the menu shows its key, and matches it exactly (F2 is not
## Shift+F2)
static func _action_shortcut(action: StringName) -> Shortcut:
    var event: InputEventAction = InputEventAction.new()
    event.action = action
    var shortcut: Shortcut = Shortcut.new()
    shortcut.events = [event]
    return shortcut


func _on_popup_menu_index_pressed(index: int) -> void:
    if index >= _windows.size():
        _menu_actions[index - _windows.size()].pressed.emit()
        return
    var win: HUDWindow = _windows[index]
    win.visible = not win.visible
    _bind_vehicle(win)


## The "View" menu: its entries show or hide the transcripts, the timetable, the scenario, all the
## control windows at once, the Lua editor, the trainset list and the driving aid
func _on_view_menu_index_pressed(index: int) -> void:
    %View.toggle_item_checked(index)
    match index:
        ViewItem.TRANSCRIPTS:
            %TranscriptsPanel.set_shown(%View.is_item_checked(index))
        ViewItem.TIMETABLE:
            %TimetablePanel.visible = %View.is_item_checked(index)
        ViewItem.SCENARIO:
            %ScenarioPanel.visible = %View.is_item_checked(index)
        ViewItem.CONTROLS:
            for win: HUDWindow in _windows:
                win.visible = %View.is_item_checked(index)
                _bind_vehicle(win)
        ViewItem.SCRIPTS:
            %ScriptEditorPanel.visible = %View.is_item_checked(index)
        ViewItem.TRAINSETS:
            %VehicleSelectorPanel.visible = %View.is_item_checked(index)
            %View.hide()
        ViewItem.DRIVING_AID:
            %DrivingAid.visible = %View.is_item_checked(index)


func _on_timetable_panel_close_requested() -> void:
    _on_view_menu_index_pressed(ViewItem.TIMETABLE)


func _on_scenario_panel_close_requested() -> void:
    _on_view_menu_index_pressed(ViewItem.SCENARIO)


func _on_script_editor_panel_close_requested() -> void:
    _on_view_menu_index_pressed(ViewItem.SCRIPTS)


func _on_vehicle_selector_panel_close_requested() -> void:
    _on_view_menu_index_pressed(ViewItem.TRAINSETS)


## A click on a selector row: the card shows the row's vehicle, on top, and the row is lit; the
## active row clicked again closes the card
func _on_vehicle_selector_panel_vehicle_activated(vehicle: RID) -> void:
    if _card and _card.vehicle == vehicle:
        _close_card()
        return
    _open_card(vehicle)


## The followed vehicle's floating button gives way to its card, where the card was last
func _on_followed_vehicle_chip_pressed() -> void:
    _open_card(%FollowedVehicleChip.vehicle)


## The card shows the vehicle, on top, and its row is lit; a card opened anew stands where the last
## one was closed, and takes the place of the followed vehicle's floating button. A click on a
## vehicle's model in free camera opens it too (SceneryHUDMouseServer.vehicle_pressed)
func _open_card(vehicle: RID) -> void:
    if not _card:
        _card = VEHICLE_CARD.instantiate()
        _card.close_requested.connect(_close_card)
        _card.remove_trainset_requested.connect(_remove_trainset)
        var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
        if player:
            _card.follow_requested.connect(player.follow_vehicle)
            _card.follow_stopped.connect(player.leave_external_view)
            _card.show_requested.connect(player.show_vehicle)
            _card.enter_requested.connect(player.enter_vehicle)
            _card.show_followed_vehicle(player.get_external_view_vehicle())
            _card.show_player_vehicle(_player_vehicle_rid(player))
        %VehicleCards.add_child(_card)
        if _card_rect.has_area():
            _card.position = _card_rect.position
            _card.size = _card_rect.size
        %FollowedVehicleChip.show_vehicle(RID())
    %VehicleCards.move_child(_card, -1)
    _card.show_vehicle(vehicle)
    %VehicleSelectorPanel.show_active_vehicle(vehicle)


func _on_player_external_view_changed(vehicle: RID) -> void:
    if _card:
        _card.show_followed_vehicle(vehicle)
    else:
        %FollowedVehicleChip.show_vehicle(vehicle)


## The card goes, remembered where it stood, and no row is lit; a vehicle still followed becomes a
## floating button
func _close_card() -> void:
    _card_rect = _card.get_rect()
    _card.queue_free()
    _card = null
    %VehicleSelectorPanel.show_active_vehicle(RID())
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    if player:
        %FollowedVehicleChip.show_vehicle(player.get_external_view_vehicle())


## Every vehicle of the trainset freed; a player in its cab steps out first, as before a scenery
## is freed - the camera lives in the vehicle's cabin (MaszynaPlayer.clear_start_train())
func _remove_trainset(vehicle: RID) -> void:
    var trainset: Array[RID] = RailVehicleServer.vehicle_get_coupled(
            vehicle, FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER)
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    if player and player.controlled_vehicle and trainset.has(player.controlled_vehicle.get_rid()):
        await player.clear_start_train()
    for trainset_vehicle: RID in trainset:
        # the wrapper goes with the RailVehicle3D it built; freed alone, the wrapper builds it again
        var wrapper: MaszynaRailVehicle3D = VehicleCard.legacy_vehicle(trainset_vehicle)
        var node: Node = wrapper if wrapper else instance_from_id(
                RailVehicleServer.vehicle_get_rail_vehicle(trainset_vehicle)) as Node
        if node:
            node.queue_free()


func _on_vehicle_freed(vehicle: RID) -> void:
    if _card and _card.vehicle == vehicle:
        _close_card()
    if %FollowedVehicleChip.vehicle == vehicle:
        %FollowedVehicleChip.show_vehicle(RID())
    if %PlayerVehicleChip.vehicle == vehicle:
        %PlayerVehicleChip.show_vehicle(RID())
    if %DrivingAid.vehicle == vehicle:
        %DrivingAid.show_vehicle(RID())


## The script context of the scenario being played, for the Lua editor; an invalid RID while none is
func attach_script_context(context: RID) -> void:
    %ScriptEditorPanel.attach_context(context)


## The scenario the player has started, for the "Scenario" entry of the View menu - hidden until
## the player opens it
func show_scenario(info: MaszynaSceneryInfo, train_id: String) -> void:
    %ScenarioPanel.show_scenario(info, train_id)


## The timetable shown is the one of the trainset the player's vehicle belongs to
func _on_controlled_vehicle_changed(player: MaszynaPlayer) -> void:
    var vehicle: RailVehicle3D = player.controlled_vehicle
    %TimetablePanel.follow_vehicle(vehicle.get_rid() if vehicle else RID())
    %DrivingAid.show_vehicle(vehicle.get_rid() if vehicle else RID())
    %VehicleSelectorPanel.follow_player_vehicle(_player_vehicle_rid(player))
    if _card:
        _card.show_player_vehicle(_player_vehicle_rid(player))
    _show_player_vehicle_chip(player)


## The vehicle the player drives or last drove, or an invalid RID
static func _player_vehicle_rid(player: MaszynaPlayer) -> RID:
    var vehicle: RailVehicle3D = player.last_controlled_vehicle
    return vehicle.get_rid() if is_instance_valid(vehicle) else RID()


func _on_player_cabin_view_changed(_in_cabin: bool, player: MaszynaPlayer) -> void:
    _show_player_vehicle_chip(player)


## The vehicle the player drives, as a floating button while the player is out of its cab
func _show_player_vehicle_chip(player: MaszynaPlayer) -> void:
    %PlayerVehicleChip.show_vehicle(RID() if player.is_in_cabin_view() else _player_vehicle_rid(player))


func _on_player_vehicle_chip_pressed() -> void:
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    if player:
        player.return_to_vehicle()


func _on_player_vehicle_chip_action_pressed() -> void:
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    if player:
        player.hand_over_vehicle()


## The vehicle the player is driving, handed to every widget that shows something about it. The
## widgets used to be given a NodePath into the vehicle's own subtree and resolve it themselves,
## which reached across two scenes and could resolve before the vehicle had been built. They are
## given the vehicle itself now, and the player says when it changes.
## Each window gets the vehicle its target names (HUDWindow.vehicle_target), as a cab control does.
func _bind_vehicle(node: Node = null) -> void:
    var player: MaszynaPlayer = get_node_or_null(player_path) as MaszynaPlayer
    var vehicle: RailVehicle3D = player.controlled_vehicle if player else null
    var controller: VehicleController = vehicle.get_controller() if vehicle else null
    for window: HUDWindow in ([node] if node else _windows):
        var target: RID = CabinState.vehicle_of(controller.get_rid(), window.vehicle_target) if controller else RID()
        var target_controller: VehicleController = instance_from_id(
                RailVehicleServer.vehicle_get_controller_instance_id(target)) as VehicleController if target.is_valid() else null
        _propagate_vehicle(window, target_controller)


func _propagate_vehicle(node: Node, controller: VehicleController) -> void:
    for child: Node in node.get_children():
        if "vehicle" in child:
            child.vehicle = controller
        _propagate_vehicle(child, controller)
