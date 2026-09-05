extends Node
class_name MultiMonitorManager

var windows: Array[Window] = []
var cameras: Array[Dictionary] = []

@onready var player: Node = get_parent()
@onready var main_camera: Camera3D = get_viewport().get_camera_3d()

var _last_mode: int = -1

func _ready() -> void:
    process_priority = 100
    UserSettings.config_changed.connect(_update_windows)
    # Wait a bit for the main camera to be initialized if needed
    _update_windows()

func _exit_tree() -> void:
    _clear_windows()

func _clear_windows() -> void:
    for win in windows:
        win.queue_free()
    windows.clear()
    cameras.clear()

func _update_windows() -> void:
    var mode: int = UserSettings.get_setting("window", "multi_monitor_mode", 0)
    
    # Ensure windows are not embedded
    var main_vp: Viewport = get_viewport()
    main_vp.gui_embed_subwindows = false
    
    if mode > 0:
        var main_win: Window = main_vp.get_window()
        if main_win:
            # Only maximize if not already in a windowed mode
            if main_win.mode != Window.MODE_WINDOWED:
                main_win.mode = Window.MODE_MAXIMIZED
    
    if mode == _last_mode:
        # Just sync settings for existing windows
        for win: Window in windows:
            _sync_window_settings(win)
        return
    
    _last_mode = mode
    _clear_windows()
    
    if mode == 0:
        return

    var screen_count: int = DisplayServer.get_screen_count()
    # if screen_count < 2:
    #    return
        
    if mode == 1: # 2 Screens: Front, Right
        _create_window(1, 1) # monitor 1, side 1 (Right)
    elif mode == 2: # 3 Screens: Left, Front, Right
        _create_window(1 if screen_count > 1 else 0, -1) # Left
        _create_window(2 if screen_count > 2 else 0, 1) # Right

func _create_window(screen_index: int, side: int) -> void:
    var win: Window = Window.new()
    win.title = "MaSzyna - Screen " + str(screen_index) + (" (Right)" if side > 0 else " (Left)")
    
    # Set position to the target screen
    win.current_screen = screen_index
    win.mode = Window.MODE_WINDOWED
    win.size = Vector2i(1280, 720)
    
    # Share the world
    win.world_3d = get_viewport().world_3d
    
    var cam: Camera3D = Camera3D.new()
    # Copy main camera settings
    cam.fov = main_camera.fov
    cam.near = main_camera.near
    cam.far = main_camera.far
    
    win.add_child(cam)
    get_tree().root.add_child(win)
    
    # Forward input to main camera
    win.window_input.connect(_on_window_input)
    
    # Sync render settings
    _sync_window_settings(win)
    
    windows.append(win)
    cameras.append({"camera": cam, "side": side})
    
    win.show()

func _sync_window_settings(win: Window) -> void:
    var main_vp: Viewport = get_viewport()
    win.msaa_3d = main_vp.msaa_3d
    win.screen_space_aa = main_vp.screen_space_aa
    win.use_debanding = main_vp.use_debanding
    win.use_occlusion_culling = main_vp.use_occlusion_culling
    win.mesh_lod_threshold = main_vp.mesh_lod_threshold

func _on_window_input(event: InputEvent) -> void:
    get_viewport().push_input(event)

func _process(_delta: float) -> void:
    if cameras.is_empty():
        return
        
    main_camera = get_viewport().get_camera_3d()
    if not main_camera:
        return

    var main_vp: Viewport = get_viewport()
    var main_vp_size: Vector2 = main_vp.get_visible_rect().size
    
    var main_v_fov: float = 0.0
    var main_h_fov: float = 0.0
    var focal_length: float = 0.0
    
    if main_camera.keep_aspect == Camera3D.KEEP_WIDTH:
        main_h_fov = deg_to_rad(main_camera.fov)
        focal_length = (main_vp_size.x / 2.0) / tan(main_h_fov / 2.0)
        main_v_fov = 2.0 * atan((main_vp_size.y / 2.0) / focal_length)
    else:
        main_v_fov = deg_to_rad(main_camera.fov)
        focal_length = (main_vp_size.y / 2.0) / tan(main_v_fov / 2.0)
        main_h_fov = 2.0 * atan((main_vp_size.x / 2.0) / focal_length)

    for cam_info: Dictionary in cameras:
        var cam: Camera3D = cam_info["camera"]
        var side: int = cam_info["side"]
        var win: Window = cam.get_window()
        var win_size: Vector2 = Vector2(win.size)
        
        # Adjust side camera FOV to match main camera's pixel scale
        # This ensures objects have the same size across screens regardless of window resolution
        var side_v_fov: float = 2.0 * atan((win_size.y / 2.0) / focal_length)
        var side_h_fov: float = 2.0 * atan((win_size.x / 2.0) / focal_length)
        
        var rot_deg: float = 0.0
        var offset: float = 0.0
        var tilt: float = 0.0
        if side > 0:
            rot_deg = UserSettings.get_setting("window", "monitor_rotation_right", 0.0)
            offset = UserSettings.get_setting("window", "monitor_offset_right", 0.0)
            tilt = UserSettings.get_setting("window", "monitor_tilt_right", 0.0)
        else:
            rot_deg = UserSettings.get_setting("window", "monitor_rotation_left", 0.0)
            offset = UserSettings.get_setting("window", "monitor_offset_left", 0.0)
            tilt = UserSettings.get_setting("window", "monitor_tilt_left", 0.0)
            
        var angle: float = 0.0
        if rot_deg != 0.0:
            angle = deg_to_rad(rot_deg)
        else:
            # Auto-calculate angle to match edges perfectly
            angle = (main_h_fov + side_h_fov) / 2.0
            
        cam.global_transform = main_camera.global_transform
        
        # Use world-space rotation to keep the horizon level and the locomotive straight
        # across physically vertical monitors even when looking up or down.
        cam.global_rotate(Vector3.UP, -angle * side)
        
        # Apply manual tilt (roll) if specified
        if tilt != 0.0:
            cam.rotate_object_local(Vector3.FORWARD, deg_to_rad(tilt) * side)
            
        cam.fov = rad_to_deg(side_v_fov)
        cam.h_offset = main_camera.h_offset + (offset * side)
        cam.v_offset = main_camera.v_offset
        cam.near = main_camera.near
        cam.far = main_camera.far
        cam.keep_aspect = main_camera.keep_aspect
