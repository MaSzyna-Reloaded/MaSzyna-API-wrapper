#include "TrainBuffCoupl.hpp"
#include "../core/TrainSystem.hpp"

namespace godot {
    void TrainBuffCoupl::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple"), &TrainBuffCoupl::couple);
        ClassDB::bind_method(D_METHOD("decouple"), &TrainBuffCoupl::decouple);
    }

    TypedArray<TrainCommand> TrainBuffCoupl::get_supported_commands() {
        TypedArray<TrainCommand> commands = LegacyBufferCouplerModule::get_supported_commands();
        commands.append(make_train_command("buffer_couple", Callable(this, "couple")));
        commands.append(make_train_command("buffer_decouple", Callable(this, "decouple")));
        return commands;
    }

    void TrainBuffCoupl::couple() {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }

        UtilityFunctions::push_warning("[TrainBuffCoupl] Coupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        if (TrainController *train_controller_node = get_train_controller_node(); train_controller_node != nullptr) {
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
        if (TrainController *train_controller_node = get_train_controller_node(); train_controller_node != nullptr) {
            TrainSystem::get_instance()->log(
                    train_controller_node->get_train_id(),
                    GameLog::LogLevel::WARNING,
                    "[TrainBuffCoupl] Decoupling is not supported yet as it requires to handle logic between 2 vehicles simultaneously");
        }
    }
} //namespace godot
