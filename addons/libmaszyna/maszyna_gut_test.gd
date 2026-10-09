extends GutTest
class_name MaszynaGutTest

func wait_idle_frames(frames, message = ""):
    while frames > 0:
        await Engine.get_main_loop().process_frame
        frames -= 1

# Train/mover config changes (train.battery_voltage = ..., send_command(...), etc.) are only
# applied to the underlying TMoverParameters once per physics tick, inside
# TrainPhysicsServer.step_physics() — never on the idle/render frame. Use this instead of
# wait_idle_frames() whenever a test needs such a change to actually take effect and be reflected
# in train.state before asserting on it.
func wait_physics_frames(frames, message = ""):
    while frames > 0:
        await Engine.get_main_loop().physics_frame
        frames -= 1
