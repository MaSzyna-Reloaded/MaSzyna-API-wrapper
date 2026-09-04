extends MaszynaGutTest


func test_camera_reference_survives_reparenting_into_cabin() -> void:
    var player:MaszynaPlayer = load("res://addons/libmaszyna/player/player.tscn").instantiate()
    var cabin:Cabin3D = Cabin3D.new()
    add_child(player)
    add_child(cabin)
    var camera:FreeCamera3D = player.get_camera()

    player.remove_child(camera)
    cabin.add_child(camera)
    player._process(0.02)

    assert_eq(player.get_camera(), camera)
    assert_eq(camera.get_parent(), cabin)

    cabin.remove_child(camera)
    player.add_child(camera)
    remove_child(cabin)
    cabin.free()
    remove_child(player)
    player.free()
