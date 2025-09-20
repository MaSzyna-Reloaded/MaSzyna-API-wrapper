extends GutTest
class_name MaszynaGutTest

func wait_idle_frames(frames, message = ""):
    while frames > 0:
        await Engine.get_main_loop().process_frame
        frames -= 1
