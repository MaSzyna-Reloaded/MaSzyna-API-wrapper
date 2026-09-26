@tool
extends RefCounted
class_name FizImportContext

## Cross-section shared state for a single FIZ import pass, mirroring MaszynaImporterContext
## from the scenery importer. One instance per top-level FIZ file (shared across `include`d
## files).

var base_dir: String = ""
var include_depth: int = 0

var controller: VehicleController = null

## EngineType decided by Engine: (VehicleEngine.EngineType) - later sections' defaults
## (MotorParamTable row format, ReleaserPowerPosLock default, ...) depend on this.
var engine_type: int = VehicleEngine.NONE
## TrainType decided by Param. (VehicleController.TrainType) - some defaults are dt_EZT-specific.
var train_type: int = VehicleController.TRAIN_TYPE_DEFAULT
## BrakeSystem decided by Cntrl. (VehicleBrake.BrakeSystemType) - most of Cntrl.'s brake-related
## fields are only meaningful when this isn't Individual.
var brake_system: int = VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL

## Full Cntrl. key/value set, stashed by FizTrainCntrlParser for the Engine: parser to consume
## once it creates the VehicleEngine-family node (Cntrl. conventionally precedes Engine:).
var cntrl_kv: Dictionary = {}

## Full Power: key/value set, stashed by FizTrainPowerParser for the concrete engine parser to
## consume once it creates the VehicleElectricEngine-family node (Power: conventionally precedes
## Engine: in real files).
var power_kv: Dictionary = {}

## Nodes already created by earlier sections, keyed by node name (mirrors sm_42v_1.tscn naming),
## so a later section can reach/configure an earlier one (e.g. Power: configures fields on the
## engine node created by Engine:).
var parts: Dictionary = {}

## Names of recognized FIZ sections with no registered parser yet (see the `_sections` table
## in FizVehicleBuilder - `parser == null`); used to only log once per vehicle per
## section instead of once per line. This does NOT mean the destination Godot class is
## missing - most of these (Engine:, Light:, Power:, ...) map to fully-bound VehicleComponent
## classes that already exist; it only means the FIZ-parsing side for that section isn't
## written yet.
var _warned_unmapped_sections: Dictionary = {}


func add_part(part_name: String, node: VehicleComponent) -> void:
    parts[part_name] = node


func get_part(part_name: String) -> VehicleComponent:
    return parts.get(part_name, null)


func warn_unmapped_section(section: String) -> void:
    if _warned_unmapped_sections.has(section):
        return
    _warned_unmapped_sections[section] = true
    push_warning("FIZ section '%s' has no parser implementation yet - data is discarded." % section)
