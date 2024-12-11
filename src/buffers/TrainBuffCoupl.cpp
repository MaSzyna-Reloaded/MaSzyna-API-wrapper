#include "TrainBuffCoupl.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot {
    void TrainBuffCoupl::_bind_methods() {
        ClassDB::bind_method(D_METHOD("couple"), &TrainBuffCoupl::couple);
        ClassDB::bind_method(D_METHOD("decouple"), &TrainBuffCoupl::decouple);
    }

    void TrainBuffCoupl::_notification(int p_what) {
        LegacyBufferCouplerModule::_notification(p_what);

        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                Node *parent = get_parent();
                while (parent != nullptr) {
                    train_controller_node = Object::cast_to<TrainController>(parent);
                    if (train_controller_node != nullptr) {
                        break;
                    }
                    parent = parent->get_parent();
                }

                if (train_controller_node != nullptr) {
                    _register_commands();
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                if (train_controller_node != nullptr) {
                    _unregister_commands();
                }
                train_controller_node = nullptr;
            } break;
            default:
                break;
        }
    }

    void TrainBuffCoupl::_register_commands() {
        if (train_controller_node == nullptr) {
            return;
        }

        TrainSystem::get_instance()->register_command(
                train_controller_node->get_train_id(), "buffer_couple", Callable(this, "couple"));
        TrainSystem::get_instance()->register_command(
                train_controller_node->get_train_id(), "buffer_decouple", Callable(this, "decouple"));
    }

    void TrainBuffCoupl::_unregister_commands() {
        if (train_controller_node == nullptr) {
            return;
        }

        TrainSystem::get_instance()->unregister_command(
                train_controller_node->get_train_id(), "buffer_couple", Callable(this, "couple"));
        TrainSystem::get_instance()->unregister_command(
                train_controller_node->get_train_id(), "buffer_decouple", Callable(this, "decouple"));
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
