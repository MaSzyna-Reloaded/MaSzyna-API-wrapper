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
    Console.add_command("server", self.console_host_server, ["address"], 0, "Host a game server")
    Console.add_command("connect", self.console_connect_server, ["address"], 0, "Connect to a game server")
    Console.add_command("nick", self.console_set_nick, ["nick"], 1, "Set your nickname")
    Console.add_command("players", self.console_list_players, [], 0, "List all players")
    Console.add_command("say", self.console_say, ["message"], 1, "Say message")

    #TrainSystem.train_log_updated.connect(self.console_print_train_log)
    LogSystem.log_updated.connect(self.console_print_log)
    PlayerSystem.error_received.connect(func(msg): LogSystem.error(msg))


func _parse_host_ip_from_address(address:String = ""):
    var port:int
    var host:String

    if address.is_empty():
        host = ProjectSettings.get_setting("maszyna/game/multiplayer/host", "127.0.0.1")
        if not host:
            console_print_log(LogSystem.LogLevel.WARNING, "Host is not configured in ProjectSetttings")
        port = int(ProjectSettings.get_setting("maszyna/game/multiplayer/port", 9797))
        if not port:
            console_print_log(LogSystem.LogLevel.WARNING, "Port is not configured in ProjectSetttings")
    else:
        var _parts = address.split(":")
        host = _parts[0].strip_edges()
        port = int(_parts[1].strip_edges())
        if host.is_empty():
            host = ProjectSettings.get_setting("maszyna/game/multiplayer/host", "127.0.0.1")

    if not port or not host:
        console_print_log(LogSystem.LogLevel.ERROR, "Incorrect address \"%s:%s\"" % [host, port])
        return [null, null]

    return [host, port]

func console_list_players():
    for player:PlayerInfo in PlayerSystem.get_all_players():
        if player:
            if player.train_id:
                Console.print_line("%s: %s [in %s]" % [player.peer_id, player.name, player.train_id])
            else:
                Console.print_line("%s: %s" % [player.peer_id, player.name])

func console_say(message: String):
    PlayerSystem.send_message(message)

func console_set_nick(nick:String):
    PlayerSystem.nick = nick
    PlayerSystem.set_player_name(PlayerSystem.get_my_peer_id(), nick)

func console_host_server(address:String = ""):
    var host_and_port = _parse_host_ip_from_address(address)
    var host = host_and_port[0]
    var port = host_and_port[1]

    if host and port:
        var err = MultiPlayerManager.host_game(host_and_port[0], host_and_port[1])
        if err == OK:
            LogSystem.info("Server started at %s:%s" % [host, port])
        else:
            LogSystem.error("Cannot host a game server (%s)" % err)


func console_connect_server(address:String = ""):
    var host_and_port = _parse_host_ip_from_address(address)
    var host = host_and_port[0]
    var port = host_and_port[1]
    if host and port:
        var err = MultiPlayerManager.connect_game(host, port)
        if err == OK:
            LogSystem.info("Connected to the server %s:%s" % [host, port])
        else:
            LogSystem.error("Can't connect to server (%s)" % err)


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
    if loglevel >= LogSystem.LogLevel.ERROR:
        Console.print_line("[color=red]%s[/color]" % [line])
    elif loglevel == LogSystem.LogLevel.WARNING:
        Console.print_line("[color=orange]%s[/color]" % [line])
    else:
        Console.print_line("%s" % [line])

func console_get_train_state(train, key=null):
    var out = TrainSystem.get_train_state(train)
    if key:
        out = out.get(key)
    Console.print_line("%s" % [out])
