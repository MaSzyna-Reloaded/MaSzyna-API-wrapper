@tool
extends VehiclePhysicsNode
class_name FizVehiclePhysicsNode

## A vehicle built from a legacy `.fiz` file.
##
## All it does is name the file and ask FizVehicleBuilder for the vehicle's description -
## the same shape E3DModelInstance has towards E3DModelManager. Everything else about owning a
## vehicle (the handle, the components, freeing them) belongs to VehiclePhysicsNode, and the
## parsing and its on-disk cache belong to the builder.

## Base MaSzyna data path used to resolve the FIZ file, matching E3DModelInstance.data_path.
@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _request_reload()

## FIZ file name, without the ".fiz" extension, matching E3DModelInstance.model_filename.
@export var fiz_filename:String = "":
    set(x):
        if not x == fiz_filename:
            fiz_filename = x
            _request_reload()

var _reload_pending:bool = false


func _ready() -> void:
    # a FIZ describes a rail vehicle: it has a place on the route and is stepped there, with or
    # without a RailVehicle3D drawing it (the editor builds no vehicle)
    if get_vehicle_rid().is_valid():
        RailVehicleServer.vehicle_attach(get_vehicle_rid())
    _request_reload()


func _request_reload() -> void:
    if _reload_pending:
        return
    _reload_pending = true
    call_deferred("_reload")


func _reload() -> void:
    _reload_pending = false
    if not is_inside_tree():
        return
    if not fiz_filename:
        set_description(null)
        return
    set_description(FizVehicleBuilder.build_description(data_path, fiz_filename))
