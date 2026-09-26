extends "res://hud/mover_switches_section.gd"

var _doors:VehicleDoors


func _do_update():
    super._do_update()
    _doors = _component(VehicleComponentType.COMPONENT_DOORS) as VehicleDoors


func _on_refresh_timer_timeout() -> void:
    if not _doors:
        return
    %LockedLight.enabled = _doors.get_locked()
    %LeftOpenLight.color_active = Color.ORANGE if _doors.get_left_operating() else Color.LIME_GREEN
    %LeftOpenLight.enabled = _doors.get_left_open() or _doors.get_left_operating()
    %RightOpenLight.color_active = Color.ORANGE if _doors.get_right_operating() else Color.LIME_GREEN
    %RightOpenLight.enabled = _doors.get_right_open() or _doors.get_right_operating()
