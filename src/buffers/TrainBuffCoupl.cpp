#include "TrainBuffCoupl.hpp"
namespace godot {
    void TrainBuffCoupl::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple"), &TrainBuffCoupl::couple);
        ClassDB::bind_method(D_METHOD("decouple"), &TrainBuffCoupl::decouple);
    }

    void TrainBuffCoupl::_enter_tree() {
        LegacyBufferCouplerModule::_enter_tree();
        train_controller_node = Object::cast_to<TrainController>(get_legacy_rail_vehicle_node());
    }

    void TrainBuffCoupl::_exit_tree() {
        train_controller_node = nullptr;
        LegacyBufferCouplerModule::_exit_tree();
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

        UtilityFunctions::push_warning("[TrainBuffCoupl] Coupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        if (train_controller_node != nullptr) {
            TrainSystem::get_instance()->log(
                    train_controller_node->get_train_id(),
                    GameLog::LogLevel::WARNING,
                    "[TrainBuffCoupl] Coupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        }
    }

    void TrainBuffCoupl::decouple() {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }

        UtilityFunctions::push_warning("[TrainBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        if (train_controller_node != nullptr) {
            TrainSystem::get_instance()->log(
                    train_controller_node->get_train_id(),
                    GameLog::LogLevel::WARNING,
                    "[TrainBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        }
    }
} //namespace godot
