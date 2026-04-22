#include "TrainBuffCoupl.hpp"
namespace godot {
    void TrainBuffCoupl::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple"), &TrainBuffCoupl::couple);
        ClassDB::bind_method(D_METHOD("decouple"), &TrainBuffCoupl::decouple);
    }

    void TrainBuffCoupl::_initialize() {
        LegacyBufferCouplerModule::_initialize();
        train_controller = Ref<TrainController>(Object::cast_to<TrainController>(get_rail_vehicle().ptr()));
    }

    void TrainBuffCoupl::_finalize() {
        train_controller.unref();
        LegacyBufferCouplerModule::_finalize();
    }

    void TrainBuffCoupl::_register_commands() {
        command_registry["buffer_couple"] = Callable(this, "couple");
        command_registry["buffer_decouple"] = Callable(this, "decouple");
    }

    void TrainBuffCoupl::_unregister_commands() {
        command_registry.erase("buffer_couple");
        command_registry.erase("buffer_decouple");
    }

    Dictionary TrainBuffCoupl::get_supported_commands() {
        command_registry.clear();
        collecting_commands = true;
        _register_commands();
        collecting_commands = false;
        return command_registry;
    }

    void TrainBuffCoupl::couple() {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }

        UtilityFunctions::push_warning(
                "[TrainBuffCoupl] Coupling is not supported yet as it requires to handle logic between 2 vehicles "
                "simultaneously");
        if (train_controller.is_valid()) {
            GameLog::get_instance()->log(
                    GameLog::LogLevel::WARNING, "[TrainBuffCoupl] Coupling is not supported yet as it requires to "
                                                "handle logic between 2 vehicles simultaneously");
        }
    }

    void TrainBuffCoupl::decouple() {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }

        UtilityFunctions::push_warning(
                "[TrainBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles "
                "simultaneously");
        if (train_controller.is_valid()) {
            GameLog::get_instance()->log(
                    GameLog::LogLevel::WARNING,
                    "[TrainBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 "
                    "vehicles simultaneously");
        }
    }
} // namespace godot
