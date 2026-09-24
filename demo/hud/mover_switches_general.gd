extends "res://hud/mover_switches_section.gd"


func _on_refresh_timer_timeout() -> void:
    if not controller:
        return
    %Speed.value = controller.get_speed()
