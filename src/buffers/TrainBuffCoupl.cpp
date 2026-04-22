#include "TrainBuffCoupl.hpp"
namespace godot {
    void TrainBuffCoupl::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple"), &TrainBuffCoupl::couple);
        ClassDB::bind_method(D_METHOD("decouple"), &TrainBuffCoupl::decouple);
    }

    void TrainBuffCoupl::set_rail_vehicle(RailVehicle *p_rail_vehicle) {
        LegacyBufferCouplerModule::set_rail_vehicle(p_rail_vehicle);
        train_controller = Object::cast_to<TrainController>(get_legacy_rail_vehicle());
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
        if (train_controller != nullptr) {
            TrainSystem::get_instance()->log(
                    train_controller->get_train_id(),
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
        if (train_controller != nullptr) {
            TrainSystem::get_instance()->log(
                    train_controller->get_train_id(),
                    GameLog::LogLevel::WARNING,
                    "[TrainBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        }
    }
} //namespace godot
