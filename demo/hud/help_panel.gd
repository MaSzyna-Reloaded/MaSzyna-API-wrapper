extends PanelContainer

## The keys of the game, as a card of the HUD like the scenario's and the vehicle's: every input
## action with its keys, grouped (help.gd). Opened from the Simulator menu.

## The close button asks the owner of the menu to hide the panel
signal close_requested


func _on_close_button_pressed() -> void:
    close_requested.emit()
