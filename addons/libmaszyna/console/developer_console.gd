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

func console_get_config_value(train, property):
    Console.print_line("%s" % TrainSystem.get_config_property(train, property))

func console_get_config_properties(train):
    var props = TrainSystem.get_supported_config_properties(train)
    var lines = []
    for prop in props:
        lines.append("%s=%s" % [prop, TrainSystem.get_config_property(train, prop)])
    Console.print_line("%s" % "\n".join(lines))

func console_broadcast(command, p1=null, p2=null):
    #Console.print_line("Broadcasting command: %s(%s, %s)" % [command, p1, p2], true)
    TrainSystem.broadcast_command(command, p1, p2)

func console_send(train, command, p1=null, p2=null):
    TrainSystem.send_command(train, command, p1, p2)

func console_list_trains():
    Console.print_line("%s" % "\n".join(TrainSystem.get_registered_trains()))

func console_list_train_commands():
    var commands = TrainSystem.get_supported_commands()
    commands.sort()
    Console.print_line("%s" % "\n".join(commands))

func console_print_log(loglevel, line):
    if loglevel >= GameLog.LogLevel.ERROR:
        Console.print_line("[color=red]%s[/color]" % [line])
    elif loglevel == GameLog.LogLevel.WARNING:
        Console.print_line("[color=orange]%s[/color]" % [line])
    else:
        Console.print_line("%s" % [line])

func console_cabin(train, operation, control=null, value=null):
    var cab:int = int(TrainSystem.get_train_state(train).get("cabin_occupied", 1))
    if operation == "controls":
        Console.print_line("cab %d controls:\n%s\nactions: %s" % [
            cab, "\n".join(CabinSystem.get_controls(train, cab)), ", ".join(CabinSystem.ACTIONS)])
    elif operation == "state" or (operation == "get" and not control):
        Console.print_line("%s" % [CabinSystem.get_state(train, cab)])
    elif operation == "get":
        Console.print_line("%s" % [CabinSystem.get_control(train, cab, control)])
    elif not StringName(operation) in CabinSystem.ACTIONS:
        TrainSystem.log(train, GameLog.LogLevel.ERROR, "Unknown cabin operation: %s" % operation)
    elif not control:
        TrainSystem.log(train, GameLog.LogLevel.ERROR, "Cabin operation %s needs a control id" % operation)
    else:
        Console.print_line("%s" % [CabinSystem.act(train, cab, control, operation, value)])

func console_get_train_state(train, key=null):
    var out = TrainSystem.get_train_state(train)
    if key:
        out = out.get(key)
    Console.print_line("%s" % [out])
