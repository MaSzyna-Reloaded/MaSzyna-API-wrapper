extends MaszynaGutTest

## Regression test: TrainElectricEngine::_do_fetch_state_from_mover used to unconditionally
## reverse-map EnginePowerSource.RAccumulator.RechargeSource and .RPowerCable.PowerTrans, both
## of which are only initialized by _do_update_internal_mover() when power_source is the
## matching variant (Accumulator / PowerCable respectively). For any other power_source -
## notably CurrentCollector, used by every real pantograph-powered electric locomotive - those
## fields held uninitialized memory, and get_mover_state() crashed the whole process with an
## uncaught std::out_of_range from std::map::at() on the very first _process() tick.

var train: TrainController


func before_each():
    train = TrainController.new()
    add_child(train)


func after_each():
    remove_child(train)
    train.free()


func test_current_collector_power_source_does_not_crash_on_process():
    var engine := TrainElectricSeriesEngine.new()
    engine.set("power/source", TrainController.POWER_SOURCE_CURRENTCOLLECTOR)
    train.add_child(engine)
    await wait_idle_frames(3)

    assert_true(engine.get_mover_state().has("power_source"))
    assert_false(
            engine.get_mover_state().has("accumulator/recharge_source"),
            "recharge_source wasn't configured for this power source and shouldn't be reported")


func test_default_power_source_does_not_crash_on_process():
    # The compiled default (power/source == NotDefined) hits the same unconditional-read path.
    var engine := TrainElectricSeriesEngine.new()
    train.add_child(engine)
    await wait_idle_frames(3)

    assert_true(engine.get_mover_state().has("power_source"))


func test_accumulator_power_source_still_reports_recharge_source():
    var engine := TrainElectricSeriesEngine.new()
    engine.set("power/source", TrainController.POWER_SOURCE_ACCUMULATOR)
    engine.set("power/accumulator/recharge_source", TrainController.POWER_SOURCE_GENERATOR)
    train.add_child(engine)
    await wait_idle_frames(3)

    assert_eq(engine.get_mover_state().get("accumulator/recharge_source"), TrainController.POWER_SOURCE_GENERATOR)
