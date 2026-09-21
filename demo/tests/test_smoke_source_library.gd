extends MaszynaGutTest

## The emitter template grammar of data/smokesource_*.txt (particles.cpp:21-120) and the values
## E3DRenderingServer derives from it. The fixture is a copy of a real template - the game
## directory is not read.

const FIXTURE:String = "res://tests/fixtures/test_smokesource.txt"


func _parse() -> MaszynaSmokeSource:
    return SmokeSourceLibrary.parse_file(FIXTURE)


func test_parses_every_block_of_a_template() -> void:
    var source:MaszynaSmokeSource = _parse()

    assert_not_null(source, "A readable template should parse")
    assert_almost_eq(source.spawn_rate, 30.0, 0.001, "spawn_rate is a top level key")
    assert_almost_eq(source.inclination_max, 15.0, 0.001, "max_inclination comes from initializer:")
    assert_almost_eq(source.velocity_min, 0.25, 0.001, "min_velocity comes from initializer:")
    assert_almost_eq(source.velocity_max, 1.7, 0.001, "max_velocity comes from initializer:")
    assert_almost_eq(source.size_min, 0.2, 0.001, "min_size comes from initializer:")
    assert_almost_eq(source.opacity_max, 0.6, 0.001, "max_opacity comes from initializer:")
    assert_eq(source.color, Color8(50, 56, 59), "color: is a 0-255 RGB triple")
    assert_almost_eq(source.size_step, 0.8, 0.001, "size_change: has its own step")
    assert_almost_eq(source.size_limit_max, 40.0, 0.001, "size_change: has its own max")
    assert_almost_eq(source.opacity_step, -0.15, 0.001, "opacity_change: step is the fade")


func test_derives_the_lifetime_and_the_particle_budget() -> void:
    var source:MaszynaSmokeSource = _parse()

    # a particle dies when its opacity reaches zero (particles.cpp:132)
    assert_almost_eq(source.get_particle_lifetime(), 4.0, 0.001, "0.6 opacity faded at 0.15 per second")
    assert_eq(source.get_particle_amount(500), 120, "30 particles per second over one lifetime")
    assert_eq(source.get_particle_amount(64), 64, "the budget is capped")


func test_terminal_size_follows_the_linear_growth() -> void:
    var source:MaszynaSmokeSource = _parse()

    # mean initial size 0.4, growing by 0.8 per second over 4 seconds, under the 40.0 limit
    assert_almost_eq(source.get_terminal_size(), 3.6, 0.001, "size grows linearly over the lifetime")

