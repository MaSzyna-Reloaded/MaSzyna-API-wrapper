@tool
extends Node

## Side views ("profiles") of vehicles and their skins, rendered from the vehicle models. The
## original starter has its own images in textures/mini, but they cover only a fraction of the
## skins (e.g. dynamic/pkp/4e_v1 has three for over forty) and look nothing like the game.
##
## Rendered profiles go into a ResourceCache, so a skin is only ever rendered once. Reading and
## writing that cache and loading the vehicle model run on SceneryLoadingTaskQueue workers; the
## render itself has to stay on the main thread, a SubViewport is only drawn with the frame.

const PROFILE_SIZE:Vector2i = Vector2i(360, 80)
## Bump to re-render the cached profiles after changing how they are rendered
const PROFILE_VERSION:int = 5
## Kept around the trimmed vehicle, so a glow drawn around it has somewhere to go
const PROFILE_MARGIN:int = 8
const CACHE_DIRECTORY:String = "vehicle_profiles"

var _cache:ResourceCache = ResourceCache.create(CACHE_DIRECTORY)
var _profiles:Dictionary[String, Texture2D] = {}
var _viewport:SubViewport
var _environment:Environment
var _camera:Camera3D
var _model_root:Node3D
var _model:E3DModelInstance
## data_path and model of the vehicle currently in the render viewport
var _model_key:String = ""
var _rendering:bool = false
var _queue:SceneryLoadingTaskQueue = SceneryLoadingTaskQueue.new()


func clear_cache() -> void:
    _cache.clear()
    _profiles.clear()


## Side view of the vehicle with the given skin, null when its model cannot be read.
## Await it: a profile missing from the caches is rendered, which takes a frame.
func get_profile(data_path:String, file_name:String, skin:String) -> Texture2D:
    # callers spell the path with or without the leading slash, it is the same vehicle
    var key:String = "%s:%s" % [data_path.to_lower().trim_prefix("/"), skin.to_lower()]
    if _profiles.has(key):
        return _profiles[key]

    var cache_path:String = _get_cache_path(key)
    var cached:Texture2D = await _run_in_queue(_cache.get.bind(cache_path)) as Texture2D
    if cached:
        _profiles[key] = cached
        return cached

    var rendered:Texture2D = await _render_profile(data_path, file_name, skin)
    if rendered:
        _profiles[key] = rendered
        _queue.submit(_cache.set.bind(cache_path, rendered, ""))
    return rendered


## Runs task on a queue worker while the main thread keeps drawing
func _run_in_queue(task:Callable) -> Variant:
    var task_id:int = _queue.submit(task)
    while not _queue.is_done(task_id):
        await get_tree().process_frame
    return _queue.wait(task_id)


## One render at a time - the viewport holds a single model, only its skin changes between shots
func _render_profile(data_path:String, file_name:String, skin:String) -> Texture2D:
    while _rendering:
        await get_tree().process_frame
    _rendering = true
    var texture:Texture2D = null
    if await _build_model(data_path, file_name, skin):
        # the vehicle over black and over white gives its real transparency, whatever the
        # viewport does with its own background
        var over_black:Image = await _capture(Color.BLACK)
        var over_white:Image = await _capture(Color.WHITE)
        var image:Image = _extract_alpha(over_black, over_white)
        # trim the empty space around the vehicle, so the profile is exactly as long as it is
        var used:Rect2i = image.get_used_rect()
        if used.size.x > 0 and used.size.y > 0:
            image = image.get_region(used.grow(PROFILE_MARGIN).intersection(Rect2i(Vector2i.ZERO, image.get_size())))
        texture = ImageTexture.create_from_image(image)
    _rendering = false
    return texture


func _capture(background:Color) -> Image:
    _environment.background_color = background
    _viewport.render_target_update_mode = SubViewport.UPDATE_ONCE
    await RenderingServer.frame_post_draw
    return _viewport.get_texture().get_image()


## What is opaque looks the same over both backgrounds, what is not lets the background through -
## the difference is the transparency (classic two-pass alpha, the render itself has none)
static func _extract_alpha(over_black:Image, over_white:Image) -> Image:
    var size:Vector2i = over_black.get_size()
    var image:Image = Image.create_empty(size.x, size.y, false, Image.FORMAT_RGBA8)
    for y:int in size.y:
        for x:int in size.x:
            var black:Color = over_black.get_pixel(x, y)
            var white:Color = over_white.get_pixel(x, y)
            var alpha:float = 1.0 - clampf(
                ((white.r - black.r) + (white.g - black.g) + (white.b - black.b)) / 3.0, 0.0, 1.0
            )
            if alpha <= 0.004:
                continue
            # over black the colour is already multiplied by its own alpha
            image.set_pixel(x, y, Color(black.r / alpha, black.g / alpha, black.b / alpha, alpha))
    return image


## The vehicle's exterior model with the given skin, seen from the side by an orthogonal camera.
## Only the skin changes between the profiles of one vehicle.
func _build_model(data_path:String, file_name:String, skin:String) -> bool:
    # every vehicle path is used with a leading slash (see maszyna_rail_vehicle_3d_instancer.gd)
    var normalized_data_path:String = data_path if data_path.begins_with("/") else "/" + data_path
    var abs_mmd_path:String = (
        UserSettings.get_maszyna_game_dir().path_join(normalized_data_path).path_join(file_name + ".mmd")
    )
    var body_model:String = MmdCabinInstancer.parse_body_model(abs_mmd_path)
    if not body_model:
        body_model = file_name
    body_model = MmdCabinInstancer.resolve_model_case(normalized_data_path, body_model)

    var skins:Array = MmdCabinInstancer.resolve_skins(normalized_data_path, skin)
    var model_key:String = "%s:%s" % [normalized_data_path, body_model]
    if _model and _model_key == model_key:
        _model.skins = skins
        _model.reload()
        return true

    # the E3D is read on a worker, only building the instance needs the main thread
    var e3d_model:E3DModel = (
        await _run_in_queue(E3DModelManager.load_model.bind(normalized_data_path, body_model)) as E3DModel
    )
    if not e3d_model:
        return false

    _ensure_viewport()
    if _model:
        _model.queue_free()
    _model = E3DModelInstance.new()
    _model.instancer = E3DModelInstance.Instancer.OPTIMIZED
    _model.data_path = normalized_data_path
    _model.model = e3d_model
    # before entering the tree: the instance builds its materials there, an empty skin list
    # makes every submodel with a dynamic material complain
    _model.skins = skins
    _model_root.add_child(_model)
    _model_key = model_key

    var bounds:AABB = _model.submodels_aabb
    if not bounds.size.length() > 0.0:
        return false
    _model.position = -bounds.get_center()
    # the vehicle runs along Z in the model frame, the camera looks at it down -X
    _camera.position = Vector3(maxf(bounds.size.z, 1.0), 0.0, 0.0)
    _camera.look_at(Vector3.ZERO)
    # an orthogonal camera sizes its view by height, a profile is much wider than it is tall
    var aspect:float = float(PROFILE_SIZE.x) / float(PROFILE_SIZE.y)
    _camera.size = maxf(bounds.size.y * 1.15, bounds.size.z * 1.05 / aspect)
    return true


func _ensure_viewport() -> void:
    if _viewport:
        return
    _viewport = SubViewport.new()
    _viewport.size = PROFILE_SIZE
    _viewport.own_world_3d = true
    _viewport.transparent_bg = true
    _viewport.msaa_3d = Viewport.MSAA_4X
    _viewport.render_target_update_mode = SubViewport.UPDATE_DISABLED
    add_child(_viewport)

    _environment = Environment.new()
    # a flat background of a known colour: the profile is cut out of two of them
    _environment.background_mode = Environment.BG_COLOR
    _environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    _environment.ambient_light_color = Color(0.45, 0.55, 0.72)
    _environment.ambient_light_energy = 0.7
    # the two passes are compared channel by channel, a tonemap would bend that comparison
    _environment.tonemap_mode = Environment.TONE_MAPPER_LINEAR
    var world_environment := WorldEnvironment.new()
    world_environment.environment = _environment
    _viewport.add_child(world_environment)

    var key_light := DirectionalLight3D.new()
    key_light.rotation_degrees = Vector3(-35.0, 40.0, 0.0)
    key_light.light_energy = 1.6
    _viewport.add_child(key_light)
    var fill_light := DirectionalLight3D.new()
    fill_light.rotation_degrees = Vector3(-20.0, -140.0, 0.0)
    fill_light.light_color = Color(0.72, 0.8, 1.0)
    fill_light.light_energy = 0.6
    _viewport.add_child(fill_light)

    _model_root = Node3D.new()
    _viewport.add_child(_model_root)
    _camera = Camera3D.new()
    _camera.projection = Camera3D.PROJECTION_ORTHOGONAL
    _viewport.add_child(_camera)


static func _get_cache_path(key:String) -> String:
    return ("%s:%d:%d" % [key, PROFILE_VERSION, E3DModel.FORMAT_VERSION]).md5_text() + ".res"
