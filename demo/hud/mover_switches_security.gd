extends "res://hud/mover_switches_section.gd"

var _security:VehicleSecuritySystem


func _do_update():
    super._do_update()
    _security = _component(VehicleComponentType.COMPONENT_SECURITY) as VehicleSecuritySystem


func _on_refresh_timer_timeout() -> void:
    if not _security:
        return
    %SecurityLight.enabled = _security.get_blinking()
    %CabSignalLight.enabled = _security.get_cabsignal_blinking()
