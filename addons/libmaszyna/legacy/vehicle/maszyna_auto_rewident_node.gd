extends Node
class_name MaszynaAutoRewidentNode

## Automatic trainset inspection of the vehicle's driver - the original TController::AutoRewident()
## (Driver.cpp:2147-2250). The wrapper has no driver (TController) layer, so this node carries that one
## piece of it: a vehicle placed with a driver (headdriver/reardriver) prepares its train once its
## engine is ready, and again after every trainset change, setting the brake delay (G/P/R) of every
## vehicle for the kind of train and releasing their manual and spring brakes.
##
## In the original that happens also with a human driver: the scenery gives the driver the
## Prepare_engine order and then Shunt/Obey_train (OrdersInit), PrepareEngine() completes once the
## engine is ready, and CheckVehicles() calls AutoRewident() in those orders (Driver.cpp:2527-2530) -
## trainset changes call CheckVehicles() as well (Train.cpp:6224, 6246). Vehicles placed without a
## speed start braked with a full manual brake (CheckLocomotiveParameters, Mover.cpp:8946), so
## without this the wagons of such a trainset never get released.
##
## Added to every vehicle by MaszynaRailVehicle3DInstancer; inactive without a driver aboard.

## bdelay_* brake delay flags (hamulce.h:49-51)
const BDELAY_G:int = 1
const BDELAY_P:int = 2
const BDELAY_R:int = 4
## AutoRewident's "passenger train" marker added to the chosen setting
const PASSENGER_TRAIN:int = 16
## Main reservoir pipe pressure PrepareEngine() waits for (Driver.cpp, isready)
const READY_FEED_PIPE_PRESSURE:float = 4.5


## In the original this is not polled at all: AutoRewident() runs inside CheckVehicles()
## (Driver.cpp:2528) for the driving orders (Shunt / Loose_shunt / Obey_train / Bank), and
## CheckVehicles() itself is called on events - an order change, PrepareEngine() completing
## (Driver.cpp:2142), a direction change, a coupling change (Driver.cpp:2622).
##
## The wrapper has no driver (TController) layer with orders to hook into, so the only event it can
## use is RailVehicleServer's vehicle_trainset_changed. What is left to poll is the engine becoming ready,
## which is a threshold the original's AI watches in its own update too - and this timer stops for
## good as soon as that happens, so a prepared vehicle costs nothing until something couples to it.
const CHECK_INTERVAL:float = 0.5


var _timer:Timer


func _ready() -> void:
    if Engine.is_editor_hint():
        return
    _timer = Timer.new()
    _timer.wait_time = CHECK_INTERVAL
    _timer.autostart = true
    _timer.timeout.connect(_check_trainset)
    add_child(_timer)


## Subscribed while in the tree - "Edit FIZ" takes a vehicle out and puts it back, and _ready()
## runs only the first time
func _enter_tree() -> void:
    if not Engine.is_editor_hint():
        RailVehicleServer.vehicle_trainset_changed.connect(_on_vehicle_trainset_changed)


## Leaving the tree ends the subscription: a neighbour freed with the scenery uncouples
## (MoverRailVehicleController::release()) after this node is already out of it
func _exit_tree() -> void:
    if not Engine.is_editor_hint():
        RailVehicleServer.vehicle_trainset_changed.disconnect(_on_vehicle_trainset_changed)


func _check_trainset() -> void:
    var parent_vehicle:RailVehicle3D = get_parent() as RailVehicle3D
    var vehicle:RID = parent_vehicle.get_rid() if parent_vehicle else RID()
    if not VehicleServer.vehicle_is_simulation_ready(vehicle):
        return
    # a vehicle without a cab has no driver to inspect its trainset, and the driver_type comes from the
    # FIZ - it will not become one later, so there is nothing left for this node to watch
    if VehicleServer.vehicle_get_driver_type(vehicle) == VehicleController.DRIVER_NOBODY:
        _timer.stop()
        return

    # PrepareEngine() completes once the engine reports ready, which is a threshold the original
    # AI watches in its own update - the only thing left worth polling for
    if not _is_engine_ready(vehicle):
        return
    _rewident(vehicle, _get_trainset(vehicle))
    # from here the trainset can only change by coupling, and that arrives as a signal
    _timer.stop()


## A vehicle joined or left the trainset (VehicleController::couple()/uncouple()), the case the
## original handles with CheckVehicles() (Driver.cpp:2622) - inspect it again once the engine of
## the new trainset reports ready.
func _on_vehicle_trainset_changed(vehicle:RID) -> void:
    var parent_vehicle:RailVehicle3D = get_parent() as RailVehicle3D
    if parent_vehicle and parent_vehicle.get_rid() == vehicle:
        _timer.start()


## The readiness condition of TController::PrepareEngine() (isready). Quirk: the converter and
## compressor terms are left out - the wrapper doesn't expose whether a vehicle has them.
func _is_engine_ready(vehicle:RID) -> bool:
    var brake:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
            vehicle, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
    var engine:RailVehicleEngine = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleEngine
    var brake_handle_ready:bool = (
            brake == null
            or not int(brake.get_controller_position())
                    == int(brake.get_handle_position(RailVehicleBrake.HANDLE_POSITION_CUTOFF)))
    return (
            not VehicleServer.vehicle_get_controller(vehicle).get_direction() == 0
            and engine != null and engine.get_main_switch_enabled()
            and (brake == null or brake.get_feed_pipe_pressure() > READY_FEED_PIPE_PRESSURE
                    or is_zero_approx(brake.tank_volume_main))
            and brake_handle_ready)


## Coupled vehicles from the head of the train (in the driving direction, CheckVehicles()) to its tail.
func _get_trainset(vehicle:RID) -> Array[RID]:
    var driving_sign:int = (VehicleServer.vehicle_get_occupied_cab(vehicle)
            * VehicleServer.vehicle_get_controller(vehicle).get_direction())
    var trainset:Array[RID] = []
    trainset.assign(RailVehicleServer.vehicle_get_coupled(
            vehicle, RailVehicleController.COUPLER_END_FRONT if driving_sign >= 0 else RailVehicleController.COUPLER_END_REAR,
            RailVehicleController.COUPLING_FLAG_COUPLER))
    return trainset


## TController::AutoRewident() (Driver.cpp:2147-2246). The driver's own vehicle is left alone, as the
## original does with a human controlled vehicle.
func _rewident(vehicle:RID, trainset:Array[RID]) -> void:
    var express:int = 0
    var freight:int = 0
    var passenger:int = 0
    var length:float = 0.0
    var mass:float = 0.0
    for member:RID in trainset:
        var member_controller:VehicleController = VehicleServer.vehicle_get_controller(member)
        length += member_controller.dimensions_length
        mass += member_controller.get_mass_total()
        if member_controller.power < 1.0:
            var member_brake:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
                    member, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
            var delays:int = member_brake.cntrl_brake_delays if member_brake else 0
            if delays & BDELAY_R:
                express += 1
            elif delays & BDELAY_G:
                freight += 1
            else:
                passenger += 1

    var setting:int
    if express + freight + passenger == 0:
        setting = PASSENGER_TRAIN + BDELAY_R # light engine
    elif freight < mini(4, express + passenger):
        setting = PASSENGER_TRAIN + (BDELAY_P if freight and express < freight + passenger else BDELAY_R)
    elif length < 300.0 and mass < 600000.0:
        setting = BDELAY_P
    elif length < 500.0 and mass < 1300000.0:
        setting = BDELAY_R
    else:
        setting = BDELAY_G

    var near_locomotive:int = 0
    for member:RID in trainset:
        var member_brake:RailVehicleBrake = RailVehicleServer.vehicle_component_get(
                member, RailVehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake
        var is_locomotive:bool = VehicleServer.vehicle_get_controller(member).power > 1.0
        var delays:int = member_brake.cntrl_brake_delays if member_brake else 0
        var brake_delay:int = BDELAY_P
        match setting:
            BDELAY_P:
                # freight P - locomotive on G, the rest on P
                brake_delay = BDELAY_G if is_locomotive else BDELAY_P
            BDELAY_G:
                # freight G - everything on G, P without it
                brake_delay = BDELAY_G if delays & BDELAY_G else BDELAY_P
            BDELAY_R:
                # freight GP - locomotive and 5 vehicles next to it on G, the rest on P
                if is_locomotive:
                    brake_delay = BDELAY_G
                    near_locomotive = 0
                else:
                    near_locomotive += 1
                    brake_delay = BDELAY_G if near_locomotive <= 5 else BDELAY_P
            PASSENGER_TRAIN + BDELAY_R:
                # passenger R - R, P without it
                brake_delay = BDELAY_R if delays & BDELAY_R else BDELAY_P
            PASSENGER_TRAIN + BDELAY_P:
                brake_delay = BDELAY_P
        if not member == vehicle:
            VehicleServer.vehicle_send_command(member, "auto_rewident", brake_delay)
