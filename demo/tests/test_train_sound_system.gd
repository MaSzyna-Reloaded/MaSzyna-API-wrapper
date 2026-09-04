extends MaszynaGutTest


func test_engine_gain_uses_rpm_and_load_without_synthetic_sound_state() -> void:
    var controller:TrainController = TrainController.new()
    controller.set_power(1000.0)
    var runtime:TrainSoundSystem.BankRuntime = TrainSoundSystem.BankRuntime.new()
    runtime.controller = controller
    var source:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    source.amplitude_offset = 0.5
    source.amplitude_factor = 1.5
    var state:Dictionary = {
        "engine_rpm_ratio": 0.8,
        "engine_power": 500.0,
    }

    # level = 0.75 * 0.8 + 0.25 * 0.5 = 0.725
    assert_almost_eq(TrainSoundSystem._engine_gain(runtime, state, source), 1.5875, 0.001)

    controller.free()


func test_engine_gain_clamps_to_event_modulation_domain() -> void:
    var controller:TrainController = TrainController.new()
    controller.set_power(1000.0)
    var runtime:TrainSoundSystem.BankRuntime = TrainSoundSystem.BankRuntime.new()
    runtime.controller = controller
    var source:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    source.amplitude_offset = 0.5
    source.amplitude_factor = 2.0

    assert_almost_eq(TrainSoundSystem._engine_gain(
            runtime, {"engine_rpm_ratio": 2.0, "engine_power": 4000.0}, source), 2.0, 0.001)
    assert_almost_eq(TrainSoundSystem._engine_gain(
            runtime, {"engine_rpm_ratio": -1.0, "engine_power": -100.0}, source), 0.5, 0.001)

    controller.free()


func test_internal_sound_is_silent_without_an_occupied_listener_vehicle() -> void:
    var runtime:TrainSoundSystem.BankRuntime = TrainSoundSystem.BankRuntime.new()
    var source:MmdSoundSourceDefinition = MmdSoundSourceDefinition.new()
    source.placement = &"internal"

    assert_almost_eq(TrainSoundSystem._soundproofing(runtime, source), 0.0, 0.001)
