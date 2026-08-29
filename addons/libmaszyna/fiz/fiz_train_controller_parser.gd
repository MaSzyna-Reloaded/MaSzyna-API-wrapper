@tool
extends RefCounted
class_name FizTrainControllerParser

## Param. and Dimensions: sections, plus the TrainController-relevant subset of Cntrl.
## (see fiz_train_cntrl_parser.gd, which delegates the controller-relevant keys here via
## apply_cntrl()). Configures `context.controller` directly rather than creating a new part.
## LoadFIZ_Param: Mover.cpp:10244, LoadFIZ_Dimensions: Mover.cpp:10361.
##
## Setters are only called when the corresponding FIZ key is present - TrainController's own
## compiled-in property defaults already match the FIZ format's "key absent" behavior except
## where noted (Cx, Floor, GroundRelayStart), so there is no need to re-specify them here.

const _CATEGORY_MAP := {
    "train": TrainController.CATEGORY_TRAIN,
    "road": TrainController.CATEGORY_ROAD,
    "unimog": TrainController.CATEGORY_ROAD,
    "ship": TrainController.CATEGORY_SHIP,
    "airplane": TrainController.CATEGORY_AIRPLANE,
}

const _TRAIN_TYPE_MAP := {
    "pseudodiesel": TrainController.TRAIN_TYPE_PSEUDODIESEL,
    "ezt": TrainController.TRAIN_TYPE_EZT,
    "dmu": TrainController.TRAIN_TYPE_DMU,
    "sn61": TrainController.TRAIN_TYPE_SN61,
    "et22": TrainController.TRAIN_TYPE_ET22,
    "et40": TrainController.TRAIN_TYPE_ET40,
    "et41": TrainController.TRAIN_TYPE_ET41,
    "et42": TrainController.TRAIN_TYPE_ET42,
    "ep05": TrainController.TRAIN_TYPE_EP05,
    "181": TrainController.TRAIN_TYPE_181,
    # "182" has no corresponding dt_* constant in this port's MOVER.h - left unmapped.
}


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    if kv.has("M") or kv.has("PWR") or kv.has("Vmax") or kv.has("Category") or kv.has("Type"):
        _parse_param(kv, context)
    if kv.has("L") or kv.has("H") or kv.has("W") or kv.has("Cx") or kv.has("Floor"):
        _parse_dimensions(kv, context)


func _parse_param(kv: Dictionary, context: FizImportContext) -> void:
    var controller: TrainController = context.controller
    if kv.has("M"):
        controller.set_mass(FizLineUtil.get_float(kv, "M"))
    if kv.has("Mred"):
        controller.set_reduced_mass(FizLineUtil.get_float(kv, "Mred"))
    if kv.has("Vmax"):
        controller.set_max_velocity(FizLineUtil.get_float(kv, "Vmax"))
    if kv.has("PWR"):
        controller.set_power(FizLineUtil.get_float(kv, "PWR"))
    if kv.has("SandCap"):
        controller.set_sand_capacity(FizLineUtil.get_float(kv, "SandCap"))
    if kv.has("HeatingP"):
        controller.set_heating_power(FizLineUtil.get_float(kv, "HeatingP"))
    if kv.has("LightP"):
        controller.set_light_power(FizLineUtil.get_float(kv, "LightP"))

    if kv.has("Category"):
        var category_str: String = FizLineUtil.get_string(kv, "Category").to_lower()
        if _CATEGORY_MAP.has(category_str):
            controller.set_category(_CATEGORY_MAP[category_str])
        else:
            push_warning("FIZ Param.Category: unknown value '%s'" % category_str)

    # Type= has no "absent" fallback distinct from the compiled default (dt_Default == 0),
    # but context.train_type must always be set (later sections' defaults depend on it) even
    # when Type= wasn't in this line.
    var train_type: int = _TRAIN_TYPE_MAP.get(FizLineUtil.get_string(kv, "Type").to_lower(), TrainController.TRAIN_TYPE_DEFAULT)
    if kv.has("Type"):
        controller.set_train_type(train_type)
    context.train_type = train_type


func _parse_dimensions(kv: Dictionary, context: FizImportContext) -> void:
    var controller: TrainController = context.controller
    if kv.has("L"):
        controller.set_length(FizLineUtil.get_float(kv, "L"))
    var height: float = FizLineUtil.get_float(kv, "H")
    if kv.has("H"):
        controller.set_height(height)
    if kv.has("W"):
        controller.set_width(FizLineUtil.get_float(kv, "W"))
    # Cx's FIZ-format default (0.3) differs from TrainController's compiled default (0.0).
    controller.set_drag_coefficient(FizLineUtil.get_float(kv, "Cx", 0.3))

    # Floor's default is conditional on H, which differs from the compiled default (0.96).
    var floor_height: float = height if height <= 2.0 else 0.0
    controller.set_floor_height(FizLineUtil.get_float(kv, "Floor", floor_height))


## Called by FizTrainCntrlParser with the full Cntrl. key/value set - applies only the
## TrainController-relevant subset (Mover.cpp:10707 LoadFIZ_Cntrl, general subsection).
func apply_cntrl(kv: Dictionary, context: FizImportContext) -> void:
    var controller: TrainController = context.controller
    if kv.has("AutomaticCabActivation"):
        controller.set_automatic_cab_activation(FizLineUtil.get_bool(kv, "AutomaticCabActivation"))
    if kv.has("BatteryStart"):
        controller.set_battery_start_mode(
                parse_start_mode(FizLineUtil.get_string(kv, "BatteryStart"), TrainController.START_MODE_MANUAL))

    # GroundRelayStart's default depends on TrainType (EZT), which differs from the compiled
    # default - so this one is always applied, even when the key is absent.
    var ground_relay_default: int = (
            TrainController.START_MODE_AUTOMATIC if context.train_type == TrainController.TRAIN_TYPE_EZT
            else TrainController.START_MODE_MANUAL)
    controller.set_ground_relay_start_mode(
            parse_start_mode(FizLineUtil.get_string(kv, "GroundRelayStart"), ground_relay_default))

    if kv.has("CompartmentLightsStart"):
        controller.set_compartment_lights_start_mode(
                parse_start_mode(FizLineUtil.get_string(kv, "CompartmentLightsStart"), TrainController.START_MODE_DISABLED))
    if kv.has("InactiveCabFlag"):
        controller.set_inactive_cab_flag(FizLineUtil.get_int(kv, "InactiveCabFlag"))


## Shared `...Start=` device activation mode decode (TrainController.StartMode - the enum this
## class owns; TrainEngine.StartMode is a duplicate of the same values to avoid a circular
## include, see TrainController.hpp). Used by Cntrl., Engine:, and other sections.
static func parse_start_mode(value: String, default_value: int) -> int:
    if value.is_empty():
        return default_value
    match value.to_lower():
        "disabled": return TrainController.START_MODE_DISABLED
        "manual": return TrainController.START_MODE_MANUAL
        "automatic": return TrainController.START_MODE_AUTOMATIC
        "mixed": return TrainController.START_MODE_MANUAL_WITH_AUTO_FALLBACK
        "battery": return TrainController.START_MODE_BATTERY
        "converter": return TrainController.START_MODE_CONVERTER
        "direction": return TrainController.START_MODE_DIRECTION
        _: return default_value


## Power-source decode (TrainController.TrainPowerSource - the enum this class owns).
## LoadFIZ_SourceDecode: Mover.cpp:11677. Used by Light:/Clima:/Power:.
static func parse_power_source(value: String, default_value: int = TrainController.POWER_SOURCE_NOT_DEFINED) -> int:
    if value.is_empty():
        return default_value
    match value.to_lower():
        "transducer": return TrainController.POWER_SOURCE_TRANSDUCER
        "generator": return TrainController.POWER_SOURCE_GENERATOR
        "accu", "accumulator": return TrainController.POWER_SOURCE_ACCUMULATOR
        "currentcollector": return TrainController.POWER_SOURCE_CURRENTCOLLECTOR
        "powercable": return TrainController.POWER_SOURCE_POWERCABLE
        "heater": return TrainController.POWER_SOURCE_HEATER
        "internal": return TrainController.POWER_SOURCE_INTERNAL
        "main": return TrainController.POWER_SOURCE_MAIN
        _: return TrainController.POWER_SOURCE_NOT_DEFINED


## Power-type decode (TrainController.TrainPowerType - the enum this class owns).
## LoadFIZ_PowerDecode: Mover.cpp:11668.
static func parse_power_type(value: String, default_value: int = TrainController.POWER_TYPE_NONE) -> int:
    if value.is_empty():
        return default_value
    match value.to_lower():
        "biopower": return TrainController.POWER_TYPE_BIO
        "mechpower": return TrainController.POWER_TYPE_MECH
        "electricpower": return TrainController.POWER_TYPE_ELECTRIC
        "steampower": return TrainController.POWER_TYPE_STEAM
        _: return TrainController.POWER_TYPE_NONE
