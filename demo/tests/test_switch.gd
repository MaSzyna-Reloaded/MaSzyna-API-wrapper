extends GutTest

func test_instantiate_switch():
	var switch = MaszynaSwitch3D.new()
	assert_not_null(switch)
	add_child_autofree(switch)
	
	var straight = MaszynaTrack3D.new()
	var s_curve = Curve3D.new()
	s_curve.add_point(Vector3.ZERO)
	s_curve.add_point(Vector3(0, 0, 10))
	straight.curve = s_curve
	switch.add_child(straight)
	
	var diverging = MaszynaTrack3D.new()
	var d_curve = Curve3D.new()
	d_curve.add_point(Vector3.ZERO)
	d_curve.add_point(Vector3(5, 0, 10))
	diverging.curve = d_curve
	switch.add_child(diverging)
	
	switch.straight_track = straight
	switch.diverging_track = diverging
	
	assert_eq(switch.state, MaszynaSwitch3D.SwitchState.STRAIGHT)
	assert_eq(switch.straight_track, straight)
	assert_eq(switch.diverging_track, diverging)
	
	switch.toggle()
	assert_eq(switch.state, MaszynaSwitch3D.SwitchState.DIVERGING)

func test_rail_vehicle_transition():
	var switch = MaszynaSwitch3D.new()
	add_child_autofree(switch)
	
	var straight = MaszynaTrack3D.new()
	var s_curve = Curve3D.new()
	s_curve.add_point(Vector3.ZERO)
	s_curve.add_point(Vector3(0, 0, 10))
	straight.curve = s_curve
	switch.add_child(straight)
	switch.straight_track = straight
	
	var vehicle = RailVehicle3D.new()
	add_child_autofree(vehicle)
	
	# Initial track
	var initial_track = MaszynaTrack3D.new()
	var i_curve = Curve3D.new()
	i_curve.add_point(Vector3(0,0,-10))
	i_curve.add_point(Vector3.ZERO)
	initial_track.curve = i_curve
	add_child_autofree(initial_track)
	
	# Connect initial track to switch
	initial_track.next_track = initial_track.get_path_to(switch)
	# Set switch common track
	switch.common_track = initial_track
	# Connect switch internal track back to switch (to trigger routing back to common)
	straight.previous_track = straight.get_path_to(switch)
	
	vehicle.track_path = vehicle.get_path_to(initial_track)
	vehicle.distance_on_track = 9.9
	
	# Simulate movement
	var controller = TrainController.new()
	add_child_autofree(controller)
	controller.state["velocity"] = 1.0 # 1 m/s
	vehicle.controller_path = vehicle.get_path_to(controller)
	
	# Force update to sync _track
	vehicle._process(0.0) 
	assert_eq(vehicle._track, initial_track)
	
	# Move forward by 0.2s -> distance becomes 10.1 -> should transition to straight
	vehicle._process(0.2)
	
	assert_eq(vehicle._track, straight)
	assert_almost_eq(vehicle.distance_on_track, 0.1, 0.01)

	# Now move backward back to common track
	controller.state["velocity"] = -2.0 # -2 m/s
	vehicle._process(0.1) # distance 0.1 -> 0.1 - 0.2 = -0.1
	
	assert_eq(vehicle._track, initial_track)
	# initial_track length is 10.0, so 10.0 - 0.1 = 9.9
	assert_almost_eq(vehicle.distance_on_track, 9.9, 0.01)

func test_multiple_transitions_in_one_frame():
	var v = RailVehicle3D.new()
	add_child_autofree(v)
	
	var t1 = MaszynaTrack3D.new()
	t1.curve = Curve3D.new(); t1.curve.add_point(Vector3(0,0,0)); t1.curve.add_point(Vector3(0,0,1)) # 1m
	add_child_autofree(t1)
	
	var t2 = MaszynaTrack3D.new()
	t2.curve = Curve3D.new(); t2.curve.add_point(Vector3(0,0,1)); t2.curve.add_point(Vector3(0,0,2)) # 1m
	add_child_autofree(t2)
	
	var t3 = MaszynaTrack3D.new()
	t3.curve = Curve3D.new(); t3.curve.add_point(Vector3(0,0,2)); t3.curve.add_point(Vector3(0,0,3)) # 1m
	add_child_autofree(t3)
	
	t1.next_track = t1.get_path_to(t2)
	t2.next_track = t2.get_path_to(t3)
	
	var controller = TrainController.new()
	add_child_autofree(controller)
	controller.state["velocity"] = 10.0 # Fast! 10 m/s
	v.controller_path = v.get_path_to(controller)
	v.track_path = v.get_path_to(t1)
	v.distance_on_track = 0.5
	
	v._process(0.0) # Init
	assert_eq(v._track, t1)
	
	v._process(0.2) # Moves 2m -> distance becomes 2.5 -> should be on T3
	assert_eq(v._track, t3)
	assert_almost_eq(v.distance_on_track, 0.5, 0.01)
