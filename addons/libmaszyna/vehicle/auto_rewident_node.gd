extends Node
class_name AutoRewidentNode

## Automatic consist inspection of the vehicle's driver - the original TController::AutoRewident()
## (Driver.cpp:2147-2250). The wrapper has no driver (TController) layer, so this node carries that one
## piece of it: a vehicle placed with a driver (headdriver/reardriver) prepares its train once its
## engine is ready, and again after every consist change, setting the brake delay (G/P/R) of every
## vehicle for the kind of train and releasing their manual and spring brakes.
##
## In the original that happens also with a human driver: the scenery gives the driver the
## Prepare_engine order and then Shunt/Obey_train (OrdersInit), PrepareEngine() completes once the
## engine is ready, and CheckVehicles() calls AutoRewident() in those orders (Driver.cpp:2527-2530) -
## consist changes call CheckVehicles() as well (Train.cpp:6224, 6246). Vehicles placed without a
## speed start braked with a full manual brake (CheckLocomotiveParameters, Mover.cpp:8946), so
## without this the wagons of such a consist never get released.
##
## Added to every vehicle by MaszynaRailVehicle3DInstancer; inactive without a driver (cabin_number 0).

## bdelay_* brake delay flags (hamulce.h:49-51)
const BDELAY_G:int = 1
const BDELAY_P:int = 2
const BDELAY_R:int = 4
## AutoRewident's "passenger train" marker added to the chosen setting
const PASSENGER_TRAIN:int = 16
## Main reservoir pipe pressure PrepareEngine() waits for (Driver.cpp, isready)
const READY_FEED_PIPE_PRESSURE:float = 4.5
## Safety limit of the consist walk
const MAX_CONSIST_VEHICLES:int = 256

var _controller:TrainController


## In the original this is not polled at all: AutoRewident() runs inside CheckVehicles()
## (Driver.cpp:2528) for the driving orders (Shunt / Loose_shunt / Obey_train / Bank), and
## CheckVehicles() itself is called on events - an order change, PrepareEngine() completing
## (Driver.cpp:2142), a direction change, a coupling change (Driver.cpp:2622).
##
## The wrapper has no driver (TController) layer with orders to hook into, so the only event it can
## use is TrainController's consist_changed. What is left to poll is the engine becoming ready,
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
    _timer.timeout.connect(_check_consist)
    add_child(_timer)


func _check_consist() -> void:
    var vehicle:RailVehicle3D = get_parent() as RailVehicle3D
    var controller:TrainController = vehicle.get_controller() if vehicle else null
    if not controller or not controller.is_node_ready():
        return
    # a vehicle without a cab has no driver to inspect its consist, and cabin_number comes from the
    # FIZ - it will not become one later, so there is nothing left for this node to watch
    if controller.cabin_number == 0:
        _timer.stop()
        return

    if not _controller == controller:
        _controller = controller
        controller.consist_changed.connect(_on_consist_changed)

    # PrepareEngine() completes once the engine reports ready, which is a threshold the original
    # AI watches in its own update - the only thing left worth polling for
    if not _is_engine_ready(controller):
        return
    _rewident(controller, _get_consist(controller))
    # from here the consist can only change by coupling, and that arrives as a signal
    _timer.stop()


## A vehicle joined or left the consist (TrainController::couple()/uncouple()), the case the
## original handles with CheckVehicles() (Driver.cpp:2622) - inspect it again once the engine of
## the new consist reports ready.
func _on_consist_changed() -> void:
    _timer.start()


## The readiness condition of TController::PrepareEngine() (isready). Quirk: the converter and
## compressor terms are left out - the wrapper doesn't expose whether a vehicle has them.
func _is_engine_ready(controller:TrainController) -> bool:
    var state:Dictionary = controller.state
    var config:Dictionary = controller.config
    var brake_handle_ready:bool = (
            not config.has("brakes_controller_position_cutoff")
            or not int(state.get("brake_controller_position", 0.0))
                    == int(config["brakes_controller_position_cutoff"]))
    return (
            not int(state.get("direction", 0)) == 0
            and bool(state.get("main_switch_enabled", false))
            and (float(state.get("feed_pipe_pressure", 0.0)) > READY_FEED_PIPE_PRESSURE
                    or is_zero_approx(float(config.get("brake_main_reservoir_volume", 0.0))))
            and brake_handle_ready)


## Coupled vehicles from the head of the train (in the driving direction, CheckVehicles()) to its tail.
func _get_consist(controller:TrainController) -> Array[TrainController]:
    var driving_sign:int = controller.cabin_number * int(controller.state.get("direction", 1))
    var end:int = 0 if driving_sign >= 0 else 1
    var head:TrainController = controller
    for i:int in MAX_CONSIST_VEHICLES:
        var next:TrainController = head.get_coupled_controller(end)
        if not next:
            break
        end = 1 - head.get_coupled_end(end)
        head = next

    var consist:Array[TrainController] = [head]
    end = 1 - end
    var current:TrainController = head
    for i:int in MAX_CONSIST_VEHICLES:
        var next:TrainController = current.get_coupled_controller(end)
        if not next:
            break
        end = 1 - current.get_coupled_end(end)
        current = next
        consist.append(current)
    return consist


## TController::AutoRewident() (Driver.cpp:2147-2246). The driver's own vehicle is left alone, as the
## original does with a human controlled vehicle.
func _rewident(controller:TrainController, consist:Array[TrainController]) -> void:
    var express:int = 0
    var freight:int = 0
    var passenger:int = 0
    var length:float = 0.0
    var mass:float = 0.0
    for member:TrainController in consist:
        length += float(member.config.get("length", 0.0))
        mass += float(member.state.get("mass_total", 0.0))
        if float(member.config.get("power", 0.0)) < 1.0:
            var delays:int = int(member.config.get("brake_delays", 0))
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
    for member:TrainController in consist:
        var is_locomotive:bool = float(member.config.get("power", 0.0)) > 1.0
        var delays:int = int(member.config.get("brake_delays", 0))
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
        if not member == controller:
            TrainSystem.send_command(member.train_id, "auto_rewident", brake_delay)
