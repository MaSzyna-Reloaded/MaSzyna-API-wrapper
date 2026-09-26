extends Node


@export var visible:bool = false:
    set(x):
        if Console:
            Console.set_visible(x)
        visible = x


func _ready() -> void:
    Console.control.visible = visible
    Console.add_command("broadcast", self.console_broadcast, ["command", "p1", "p2"], 1, "Broadcast message to all trains")
    Console.add_command("send", self.console_send, ["train", "command", "p1", "p2"], 2, "Send message to a train")
    Console.add_command("trains", self.console_list_trains, 0, 0, "List trains")
    Console.add_command("commands", self.console_list_train_commands, 0, 0, "List available train commands")
    Console.add_command("get", self.console_get_train_state, ["train", "parameter"], 1, "Get train state / parameter")
    Console.add_command("prop", self.console_get_config_value, ["train", "property"], 2, "Get train config property")
    Console.add_command("props", self.console_get_config_properties, ["train"], 1, "List train config properties")
    Console.add_command(
        "cabin", self.console_cabin, ["train", "operation", "control", "value"], 2,
        "Occupied cabin: controls | state | get [control] | increase|decrease|hold|release|toggle|set <control> [value]")

    GameLog.log_updated.connect(self.console_print_log)

## The console knows a vehicle by its scenery name only, so it goes through the server's name
## registry; a name that is empty, "none" or unknown reaches no vehicle.
func _vehicle(train:String) -> RID:
    var vehicle:RID = RailVehicleServer.vehicle_get_rid_by_name(train)
    if not vehicle.is_valid():
        GameLog.error("No vehicle named \"%s\"" % train)
    return vehicle

func console_get_config_value(train, property):
    var vehicle:RID = _vehicle(train)
    if vehicle.is_valid():
        Console.print_line("%s" % RailVehicleServer.vehicle_dump_config(vehicle).get(property))

func console_get_config_properties(train):
    var vehicle:RID = _vehicle(train)
    if not vehicle.is_valid():
        return
    var config:Dictionary = RailVehicleServer.vehicle_dump_config(vehicle)
    var lines = []
    for prop in config:
        lines.append("%s=%s" % [prop, config[prop]])
    Console.print_line("%s" % "\n".join(lines))

func console_broadcast(command, p1=null, p2=null):
    RailVehicleServer.broadcast_command(command, p1, p2)

func console_send(train, command, p1=null, p2=null):
    var vehicle:RID = _vehicle(train)
    if vehicle.is_valid():
        RailVehicleServer.vehicle_send_command(vehicle, command, p1, p2)

func console_list_trains():
    var names:PackedStringArray = []
    for vehicle:RID in RailVehicleServer.get_vehicles():
        names.append(RailVehicleServer.vehicle_get_name(vehicle))
    Console.print_line("%s" % "\n".join(names))

func console_list_train_commands():
    var commands:Dictionary[String, bool] = {}
    for vehicle:RID in RailVehicleServer.get_vehicles():
        for command:String in RailVehicleServer.vehicle_get_commands(vehicle):
            commands[command] = true
    var names:Array[String] = []
    names.assign(commands.keys())
    names.sort()
    Console.print_line("%s" % "\n".join(names))

func console_print_log(loglevel, line):
    if loglevel >= GameLog.LogLevel.ERROR:
        Console.print_line("[color=red]%s[/color]" % [line])
    elif loglevel == GameLog.LogLevel.WARNING:
        Console.print_line("[color=orange]%s[/color]" % [line])
    else:
        Console.print_line("%s" % [line])

func console_cabin(train, operation, control=null, value=null):
    var vehicle:RID = _vehicle(train)
    if not vehicle.is_valid():
        return
    var cab:int = int(RailVehicleServer.vehicle_dump_state(vehicle).get("cabin_occupied", 1))
    if operation == "controls":
        Console.print_line("cab %d controls:\n%s\nactions: %s" % [
            cab, "\n".join(CabinSystem.get_controls(vehicle, cab)), ", ".join(CabinSystem.ACTIONS)])
    elif operation == "state" or (operation == "get" and not control):
        Console.print_line("%s" % [CabinSystem.get_state(vehicle, cab)])
    elif operation == "get":
        Console.print_line("%s" % [CabinSystem.get_control(vehicle, cab, control)])
    elif not StringName(operation) in CabinSystem.ACTIONS:
        GameLog.error("%s: Unknown cabin operation: %s" % [train, operation])
    elif not control:
        GameLog.error("%s: Cabin operation %s needs a control id" % [train, operation])
    else:
        Console.print_line("%s" % [CabinSystem.act(vehicle, cab, control, operation, value)])

func console_get_train_state(train, key=null):
    var vehicle:RID = _vehicle(train)
    if not vehicle.is_valid():
        return
    var out = RailVehicleServer.vehicle_dump_state(vehicle)
    if key:
        out = out.get(key)
    Console.print_line("%s" % [out])
