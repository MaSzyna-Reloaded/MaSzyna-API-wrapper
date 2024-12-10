extends GutTest

var SM42_scene: PackedScene = preload('res://sm_42v_1.tscn')
var SM42: TrainController

func before_each():
	SM42 = SM42_scene.instantiate()
	add_child(SM42)
	while not SM42.is_node_ready():
		pass
	SM42.update_state()

	#TrainSystem.register_train("sm42v1", SM42)

func after_each():
	remove_child(SM42)
	SM42.free()
	SM42 = null

func test_sm42_battery():
	SM42.send_command("battery", false)
	assert_eq(SM42.state["battery_enabled"], false)
	SM42.send_command("battery", true)
	assert_eq(SM42.state["battery_enabled"], true)
