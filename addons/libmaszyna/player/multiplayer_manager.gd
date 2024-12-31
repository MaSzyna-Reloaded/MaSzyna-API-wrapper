extends Node

func host_game(ip, port) -> int:
    var peer = ENetMultiplayerPeer.new()
    peer.set_bind_ip(ip)
    var err = peer.create_server(port, 32)
    if err == OK:
        multiplayer.multiplayer_peer = peer
    return err


func connect_game(ip, port) -> int:
    var peer = ENetMultiplayerPeer.new()
    var err = peer.create_client(ip, port)
    if err == OK:
        multiplayer.multiplayer_peer = peer
    return err
