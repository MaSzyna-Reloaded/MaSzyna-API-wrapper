extends "res://hud/mover_switches_section.gd"

var _brakes:VehicleBrake


func _do_update():
    super._do_update()
    _brakes = _component(VehicleComponentType.COMPONENT_BRAKES) as VehicleBrake


func _on_refresh_timer_timeout() -> void:
    if not _brakes:
        return
    %BrakeCylinderPressure.value = _brakes.get_air_pressure()
    %BrakePipePressure.value = _brakes.get_pipe_pressure()
