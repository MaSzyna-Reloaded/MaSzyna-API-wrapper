extends "res://hud/mover_switches_section.gd"

var _spring_brake:VehicleSpringBrake


func _do_update():
    super._do_update()
    _spring_brake = _component(VehicleComponentType.COMPONENT_SPRING_BRAKE) as VehicleSpringBrake


func _on_refresh_timer_timeout() -> void:
    if not _spring_brake:
        return
    %CylinderPressure.value = _spring_brake.get_cylinder_pressure()
    %EnabledLight.enabled = not _spring_brake.get_shut_off()
    %ActiveLight.enabled = _spring_brake.get_active()
    %BrakingLight.enabled = _spring_brake.get_braking()
