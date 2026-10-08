extends GutTest

## frame_post_draw can be emitted on the render thread; GUT and nodes stay on the main thread.
signal frame_rendered

## Regression (FINDINGS.md, 2026-10-06 "Crash at 0% loading on D3D12"): gnd-skydome's sun shafts
## sampled the scene colour and wrote it in one dispatch - D3D12 removed the device - and the first
## fix copied it with texture_copy(), which the scene colour refuses (no CAN_COPY_FROM): an engine
## error every frame and shafts drawn over no scene colour.
##
## The scene is a flat green background; the effect's debug overlay mixes magenta in at half
## strength and the shafts add nothing (weight 0). A grey pixel means the effect drew over the
## scene's colour; magenta alone means it never got it.
## Odd dimensions exercise the partial compute workgroups at the image's edges.
const VIEWPORT_SIZE: Vector2i = Vector2i(65, 67)
const SECOND_VIEWPORT_SIZE: Vector2i = Vector2i(127, 73)
const BACKGROUND: Color = Color(0.0, 1.0, 0.0)
const OVERLAY: Color = Color(1.0, 0.0, 1.0)
const OVERLAY_STRENGTH: float = 0.5
const SCREEN_CENTRE_UV: Vector2 = Vector2(0.5, 0.5)
## The overlay is drawn within this distance of the sun, in screen UV - the whole viewport
const WHOLE_SCREEN_RADIUS: float = 2.0
const CHANNEL_TOLERANCE: float = 0.05
const EXCESSIVE_SAMPLE_COUNT: int = 2147483647
const MAX_SAMPLE_COUNT: int = 100

var _effect: SunShaftsCompositorEffect
var _compositor: Compositor


func before_each() -> void:
    RenderingServer.frame_post_draw.connect(frame_rendered.emit, CONNECT_DEFERRED)
    _effect = SunShaftsCompositorEffect.new()
    _effect.sun_visible = true
    _effect.sun_screen_uv = SCREEN_CENTRE_UV
    _effect.max_radius = WHOLE_SCREEN_RADIUS
    _effect.weight = 0.0
    _effect.debug_overlay_strength = OVERLAY_STRENGTH
    _effect.debug_overlay_color = OVERLAY
    var effects: Array[CompositorEffect] = [_effect]
    _compositor = Compositor.new()
    _compositor.compositor_effects = effects


func after_each() -> void:
    RenderingServer.frame_post_draw.disconnect(frame_rendered.emit)
    _compositor = null
    _effect = null


func test_sample_count_has_a_runtime_upper_bound() -> void:
    _effect.sample_count = EXCESSIVE_SAMPLE_COUNT
    assert_eq(_effect.sample_count, MAX_SAMPLE_COUNT)


func test_draws_over_the_scene_colour_without_engine_errors() -> void:
    if RenderingServer.get_rendering_device() == null:
        pending("needs a GPU renderer (no RenderingDevice in --headless)")
        return

    var viewport: SubViewport = _create_viewport(VIEWPORT_SIZE)
    await frame_rendered
    _assert_overlay(viewport)
    assert_engine_error_count(0, "the effect renders without engine errors")


func test_shared_effect_survives_viewport_resize_and_msaa() -> void:
    if RenderingServer.get_rendering_device() == null:
        pending("needs a GPU renderer (no RenderingDevice in --headless)")
        return

    _effect.sample_count = EXCESSIVE_SAMPLE_COUNT
    # UV 1.0 is the edge of the image, not a valid texel coordinate.
    _effect.sun_screen_uv = Vector2.ONE
    _effect.density = 1.0
    var first: SubViewport = _create_viewport(VIEWPORT_SIZE)
    var second: SubViewport = _create_viewport(SECOND_VIEWPORT_SIZE)
    second.msaa_3d = Viewport.MSAA_2X
    await frame_rendered
    _assert_overlay(first)
    _assert_overlay(second)
    first.size = SECOND_VIEWPORT_SIZE
    second.size = VIEWPORT_SIZE
    await frame_rendered
    _assert_overlay(first)
    _assert_overlay(second)
    assert_engine_error_count(0, "sharing, resizing and resolving preserve valid GPU resources")


func test_invisible_zero_radius_and_invalid_sun_leave_colour_untouched() -> void:
    if RenderingServer.get_rendering_device() == null:
        pending("needs a GPU renderer (no RenderingDevice in --headless)")
        return

    var viewport: SubViewport = _create_viewport(VIEWPORT_SIZE)
    _effect.sun_visible = false
    await frame_rendered
    _assert_background(viewport)
    _effect.sun_visible = true
    _effect.max_radius = 0.0
    await frame_rendered
    _assert_background(viewport)
    _effect.max_radius = WHOLE_SCREEN_RADIUS
    _effect.sun_screen_uv = Vector2(INF, NAN)
    await frame_rendered
    _assert_background(viewport)
    _effect.sun_screen_uv = SCREEN_CENTRE_UV
    await frame_rendered
    _assert_overlay(viewport)
    assert_engine_error_count(0, "inactive or invalid parameters never dispatch invalid work")


func _create_viewport(size: Vector2i) -> SubViewport:
    var environment: Environment = Environment.new()
    environment.background_mode = Environment.BG_COLOR
    environment.background_color = BACKGROUND
    var world: World3D = World3D.new()
    world.environment = environment

    var viewport: SubViewport = SubViewport.new()
    viewport.size = size
    viewport.own_world_3d = true
    viewport.world_3d = world
    viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
    var camera: Camera3D = Camera3D.new()
    camera.compositor = _compositor
    viewport.add_child(camera)
    add_child_autofree(viewport)
    return viewport


func _assert_overlay(viewport: SubViewport) -> void:
    var image: Image = viewport.get_texture().get_image()
    var pixels: Array[Vector2i] = [Vector2i.ZERO, viewport.size / 2, viewport.size - Vector2i.ONE]
    for position: Vector2i in pixels:
        var pixel: Color = image.get_pixelv(position)
        assert_gt(pixel.g, CHANNEL_TOLERANCE, "the scene's green is under the overlay: %s" % pixel)
        assert_almost_eq(pixel.g, pixel.r, CHANNEL_TOLERANCE, "half green, half magenta is grey: %s" % pixel)
        assert_almost_eq(pixel.b, pixel.r, CHANNEL_TOLERANCE, "half green, half magenta is grey: %s" % pixel)


func _assert_background(viewport: SubViewport) -> void:
    var pixel: Color = viewport.get_texture().get_image().get_pixelv(viewport.size / 2)
    assert_almost_eq(pixel.r, BACKGROUND.r, CHANNEL_TOLERANCE)
    assert_almost_eq(pixel.g, BACKGROUND.g, CHANNEL_TOLERANCE)
    assert_almost_eq(pixel.b, BACKGROUND.b, CHANNEL_TOLERANCE)
