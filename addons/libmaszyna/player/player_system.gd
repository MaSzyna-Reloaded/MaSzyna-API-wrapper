extends Node
#class_name PlayerSystem

var _players = {}
var _trains_occupied = {}  # train_id to peer_id map

var nick:String = ""
var occupied_train: String = ""


signal error_received(error: String)
signal kicked_off_train(peer_id: int, train_id: String)

func _ready():
    multiplayer.peer_connected.connect(_on_peer_connected)
    multiplayer.peer_disconnected.connect(_on_peer_disconnected)
    multiplayer.connected_to_server.connect(_on_connected_to_server)
    multiplayer.server_disconnected.connect(_on_server_disconnected)

func _on_connected_to_server():
    LogSystem.info("Connected to server successfully")

func _on_server_disconnected():
    LogSystem.info("Disconnected from server")

func _error(msg):
    error_received.emit(msg)


func _on_peer_connected(peer_id: int):
    LogSystem.info("Peer connected: %d" % peer_id)
    register_player.rpc(peer_id)
    set_player_name.rpc(peer_id, nick)
    if occupied_train:
        enter_train.rpc(peer_id, occupied_train)


func _on_peer_disconnected(peer_id: int):
    unregister_player.rpc(peer_id)


func _require_registered_player(peer_id):
    if not _players.has(peer_id):
        _error("Player not registered: %s" % peer_id)
        false
    else:
        return true

func _require_train_occupied_by_player(peer_id, train_id):
    if _require_registered_player(peer_id):
        if not train_id in _trains_occupied:
            _error("Train is not occupied: %s" % train_id)
            return false
        if not _trains_occupied.has(train_id):
            _error("Train is not occupied: %s" % train_id)
            return false
        return true
    else:
        return false


func recevie_error(message):
    error_received.emit(message)


@rpc("any_peer", "call_local")
func register_player(peer_id:int, name:String = "", type=PlayerInfo.PlayerType.PLAYER_TYPE_LOCAL):
    if _players.has(peer_id):
        _error("Player already registered: %s" % peer_id)
        return

    var p = PlayerInfo.new()
    p.name = name if not name.is_empty() else "Player %d" % peer_id
    p.type = type
    p.peer_id = peer_id

    _players[peer_id] = p


@rpc("any_peer", "call_local")
func unregister_player(peer_id:int):
    if _require_registered_player(peer_id):
        _players.erase(peer_id)


func get_all_players():
    return _players.values()


func get_my_peer_id():
    return multiplayer.get_unique_id()

@rpc("any_peer", "call_local")
func set_player_name(peer_id:int, nick: String):
    if not nick.is_empty() and _require_registered_player(peer_id):
        receive_player_name.rpc(peer_id, nick)

@rpc("any_peer", "call_local")
func receive_player_name(peer_id:int, nick: String):
    _players[peer_id].name = nick
    LogSystem.warning("Player %d changed nick to %s" % [peer_id, nick])

func get_player_name(peer_id:int):
    if _require_registered_player(peer_id):
        return _players[peer_id].name

@rpc("any_peer", "call_local")
func enter_train(peer_id:int, train_id:String):
    if _require_registered_player(peer_id):
        if _trains_occupied.has(train_id):
            _error("Train is already occupied: %s" % train_id)
            if multiplayer.get_remote_sender_id() == 1:
                #kicked_off_train.emit(peer_id, train_id)
                kick_off_from_train.rpc(peer_id, train_id)
                #handle_train_occupy_conflict.rpc_id(peer_id, train_id, _trains_occupied[train_id])
            return
        _trains_occupied[train_id] = peer_id
        _players[peer_id].train_id = train_id


@rpc("any_peer", "call_local")
func exit_train(peer_id:int, train_id:String):
    if _require_train_occupied_by_player(peer_id, train_id):
        if _trains_occupied[train_id] == peer_id:
            _trains_occupied.erase(train_id)
            _players[peer_id].train_id = ""
        else:
            _error("Train is occupied by someone else")


@rpc("any_peer", "call_local")
func kick_off_from_train(peer_id:int, train_id:String):
    kicked_off_train.emit(peer_id, train_id)

@rpc("any_peer","call_remote")
func handle_train_occupy_conflict(train_id:String, actual_peer_id:int):
    print("HERE")
    _trains_occupied[train_id] = actual_peer_id
    kicked_off_train.emit(multiplayer.get_remote_sender_id(), train_id)


func send_train_command(peer_id:int, train_id:String, command: String, p1 = null, p2 = null):
    if _require_train_occupied_by_player(peer_id, train_id):
        TrainSystem.send_command(train_id, command, p1, p2)

func send_message(message: String):
    receive_message.rpc(multiplayer.get_unique_id(), message)

@rpc("any_peer", "call_remote")
func receive_message(peer_id:int, message: String):
    LogSystem.info("%s: %s" % [get_player_name(peer_id), message])
