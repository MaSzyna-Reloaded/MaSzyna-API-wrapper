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

    TrainSystem.train_log_updated.connect(self.console_print_train_log)
    TrainSystem.log_updated.connect(self.console_print_log)

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

func console_print_train_log(train_id, loglevel, line):
    console_print_log(loglevel, "%s: %s" % [train_id, line])

func console_print_log(loglevel, line):
    if loglevel >= TrainSystem.TrainLogLevel.TRAINLOGLEVEL_ERROR:
        Console.print_line("[color=red]%s[/color]" % [line])
    elif loglevel == TrainSystem.TrainLogLevel.TRAINLOGLEVEL_WARNING:
        Console.print_line("[color=orange]%s[/color]" % [line])
    else:
        Console.print_line("%s" % [line])

func console_get_train_state(train, key=null):
    var out = TrainSystem.get_train_state(train)
    if key:
        out = out.get(key)
    Console.print_line("%s" % [out])
