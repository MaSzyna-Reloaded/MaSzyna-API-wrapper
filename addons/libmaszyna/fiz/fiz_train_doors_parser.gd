@tool
extends RefCounted
class_name FizTrainDoorsParser

## Doors: section -> TrainDoors. LoadFIZ_Doors: Mover.cpp:10537.
##
## Setters are only called when the corresponding FIZ key is present - TrainDoors' own
## compiled-in property defaults already match the FIZ format's "key absent" behavior, except
## `voltage` (default depends on open/close method) and `max_shift` (two alternate source
## keys with a priority rule), which are genuine cross-field/conditional defaults.

const _CONTROLS_MAP := {
    "passenger": TrainDoors.CONTROLS_PASSENGER,
    "automaticctrl": TrainDoors.CONTROLS_AUTOMATIC,
    "driverctrl": TrainDoors.CONTROLS_DRIVER,
    "conductor": TrainDoors.CONTROLS_CONDUCTOR,
    "mixed": TrainDoors.CONTROLS_MIXED,
}

const _TYPE_MAP := {
    "shift": TrainDoors.TYPE_SHIFT,
    "rotate": TrainDoors.TYPE_ROTATE,
    "fold": TrainDoors.TYPE_FOLD,
    "plug": TrainDoors.TYPE_PLUG,
}


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := TrainDoors.new()

    var open_method: int = _CONTROLS_MAP.get(FizLineUtil.get_string(kv, "OpenCtrl").to_lower(), TrainDoors.CONTROLS_PASSENGER)
    if kv.has("OpenCtrl"):
        node.set_open_method(open_method)
    if kv.has("CloseCtrl"):
        node.set_close_method(_CONTROLS_MAP.get(FizLineUtil.get_string(kv, "CloseCtrl").to_lower(), TrainDoors.CONTROLS_PASSENGER))

    if kv.has("DoorStayOpen"):
        node.set_open_time(FizLineUtil.get_float(kv, "DoorStayOpen"))
    if kv.has("OpenSpeed"):
        node.set_open_speed(FizLineUtil.get_float(kv, "OpenSpeed"))
    if kv.has("OpenDelay"):
        node.set_open_delay(FizLineUtil.get_float(kv, "OpenDelay"))
    if kv.has("CloseSpeed"):
        node.set_close_speed(FizLineUtil.get_float(kv, "CloseSpeed"))
    if kv.has("CloseDelay"):
        node.set_close_delay(FizLineUtil.get_float(kv, "CloseDelay"))
    if kv.has("DoorClosureWarning"):
        node.set_close_warning(FizLineUtil.get_bool(kv, "DoorClosureWarning"))
    if kv.has("DoorClosureWarningAuto"):
        node.set_auto_close_warning(FizLineUtil.get_bool(kv, "DoorClosureWarningAuto"))
    if kv.has("DoorAutoCloseRemote"):
        node.set_auto_close_remote(FizLineUtil.get_bool(kv, "DoorAutoCloseRemote"))
    if kv.has("DoorAutoCloseVel"):
        node.set_auto_close_velocity(FizLineUtil.get_float(kv, "DoorAutoCloseVel"))
    if kv.has("DoorBlocked"):
        node.set_has_lock(FizLineUtil.get_bool(kv, "DoorBlocked"))
    if kv.has("DoorOpenWithPermit"):
        node.set_open_with_permit(FizLineUtil.get_float(kv, "DoorOpenWithPermit"))

    # DoorMaxShiftL and DoorMaxShiftR both write the same field; when both are present, R wins.
    if kv.has("DoorMaxShiftL"):
        node.set_max_shift(FizLineUtil.get_float(kv, "DoorMaxShiftL"))
    if kv.has("DoorMaxShiftR"):
        node.set_max_shift(FizLineUtil.get_float(kv, "DoorMaxShiftR"))
    if kv.has("DoorMaxShiftPlug"):
        node.set_max_shift_plug(FizLineUtil.get_float(kv, "DoorMaxShiftPlug"))

    if kv.has("DoorOpenMethod"):
        node.set_type(_TYPE_MAP.get(FizLineUtil.get_string(kv, "DoorOpenMethod").to_lower(), TrainDoors.TYPE_ROTATE))

    # DoorVoltage's absent-key default depends on whether doors are remote-controlled, which
    # differs from the compiled default (0/unset) for driver/conductor/mixed doors.
    var voltage_str: String = FizLineUtil.get_string(kv, "DoorVoltage")
    if voltage_str.is_empty():
        var remote: bool = open_method in [TrainDoors.CONTROLS_DRIVER, TrainDoors.CONTROLS_CONDUCTOR, TrainDoors.CONTROLS_MIXED]
        if remote:
            node.set_voltage(TrainDoors.VOLTAGE_24)
    else:
        match voltage_str.to_int():
            12: node.set_voltage(TrainDoors.VOLTAGE_12)
            24: node.set_voltage(TrainDoors.VOLTAGE_24)
            112: node.set_voltage(TrainDoors.VOLTAGE_112)
            0: node.set_voltage(TrainDoors.VOLTAGE_0)
            _: push_warning("FIZ Doors:DoorVoltage: unexpected value '%s'" % voltage_str)

    if kv.has("DoorNeedPermit"):
        node.set_permit_required(FizLineUtil.get_bool(kv, "DoorNeedPermit"))
    var permit_list_str: String = FizLineUtil.get_string(kv, "DoorPermitList")
    if not permit_list_str.is_empty():
        var permit_list: Array = []
        for part: String in permit_list_str.split("|", false):
            permit_list.append(part.to_int())
        if not permit_list.is_empty():
            node.set_permit_list(permit_list)
            node.set_permit_default(FizLineUtil.get_int(kv, "DoorPermitListDefault", 1))
    if kv.has("DoorsPermitLightBlinking"):
        node.set_permit_light_blinking(FizLineUtil.get_int(kv, "DoorsPermitLightBlinking"))

    if kv.has("PlatformSpeed"):
        node.set_platform_speed(FizLineUtil.get_float(kv, "PlatformSpeed"))
    if kv.has("PlatformMaxShift"):
        node.set_platform_max_shift(FizLineUtil.get_float(kv, "PlatformMaxShift"))
    if kv.has("PlatformMaxSpeed"):
        node.set_platform_max_speed(FizLineUtil.get_float(kv, "PlatformMaxSpeed"))
    if kv.has("PlatformOpenMethod"):
        node.set_platform_type(
                TrainDoors.PLATFORM_TYPE_SHIFT if FizLineUtil.get_string(kv, "PlatformOpenMethod").to_lower() == "shift"
                else TrainDoors.PLATFORM_TYPE_ROTATE)

    if kv.has("MirrorMaxShift"):
        node.set_mirror_max_shift(FizLineUtil.get_float(kv, "MirrorMaxShift"))
    if kv.has("MirrorVelClose"):
        node.set_mirror_close_velocity(FizLineUtil.get_float(kv, "MirrorVelClose"))

    context.add_part("TrainDoors", node)
