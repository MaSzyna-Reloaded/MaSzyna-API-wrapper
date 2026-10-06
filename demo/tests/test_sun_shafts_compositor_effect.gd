extends MaszynaGutTest

## Regression (FINDINGS.md, 2026-10-06 "Crash at 0% loading on D3D12"): gnd-skydome's sun shafts
## sampled the scene colour and wrote it in one dispatch - D3D12 removed the device - and the first
## fix copied it with texture_copy(), which the scene colour refuses (no CAN_COPY_FROM): an engine
## error every frame and shafts drawn over no scene colour.
##
## The scene is a flat green background; the effect's debug overlay mixes magenta in at half
## strength and the shafts add nothing (weight 0). A grey pixel means the effect drew over the
## scene's colour; magenta alone means it never got it.
const VIEWPORT_SIZE: Vector2i = Vector2i(64, 64)
const BACKGROUND: Color = Color(0.0, 1.0, 0.0)
const OVERLAY: Color = Color(1.0, 0.0, 1.0)
const OVERLAY_STRENGTH: float = 0.5
const SCREEN_CENTRE_UV: Vector2 = Vector2(0.5, 0.5)
## The overlay is drawn within this distance of the sun, in screen UV - the whole viewport
const WHOLE_SCREEN_RADIUS: float = 2.0
const FRAMES_TO_RENDER: int = 4
const CHANNEL_TOLERANCE: float = 0.05


func test_draws_over_the_scene_colour_without_engine_errors() -> void:
    if RenderingServer.get_rendering_device() == null:
        pending("needs a GPU renderer (no RenderingDevice in --headless)")
        return

    var effect: SunShaftsCompositorEffect = SunShaftsCompositorEffect.new()
    effect.sun_visible = true
    effect.sun_screen_uv = SCREEN_CENTRE_UV
    effect.max_radius = WHOLE_SCREEN_RADIUS
    effect.weight = 0.0
    effect.debug_overlay_strength = OVERLAY_STRENGTH
    effect.debug_overlay_color = OVERLAY
    var effects: Array[CompositorEffect] = [effect]
    var compositor: Compositor = Compositor.new()
    compositor.compositor_effects = effects

    var environment: Environment = Environment.new()
    environment.background_mode = Environment.BG_COLOR
    environment.background_color = BACKGROUND
    var world: World3D = World3D.new()
    world.environment = environment

    var viewport: SubViewport = SubViewport.new()
    viewport.size = VIEWPORT_SIZE
    viewport.own_world_3d = true
    viewport.world_3d = world
    viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
    var camera: Camera3D = Camera3D.new()
    camera.compositor = compositor
    viewport.add_child(camera)
    add_child_autofree(viewport)

    for _frame: int in FRAMES_TO_RENDER:
        await RenderingServer.frame_post_draw

    var pixel: Color = viewport.get_texture().get_image().get_pixelv(VIEWPORT_SIZE / 2)
    assert_gt(pixel.g, CHANNEL_TOLERANCE, "the scene's green is under the overlay: %s" % pixel)
    assert_almost_eq(pixel.g, pixel.r, CHANNEL_TOLERANCE, "half green, half magenta is grey: %s" % pixel)
    assert_almost_eq(pixel.b, pixel.r, CHANNEL_TOLERANCE, "half green, half magenta is grey: %s" % pixel)
    assert_engine_error_count(0, "the effect renders without engine errors")
