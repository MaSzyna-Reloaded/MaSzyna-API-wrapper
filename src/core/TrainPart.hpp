#pragma once
#include "./GameLog.hpp"
#include "./TrainSystem.hpp"
#include "TrainController.hpp"
#include <functional>
#include <godot_cpp/classes/node.hpp>

#define ASSERT_MOVER(mover_ptr)                                                                                        \
    if ((mover_ptr) == nullptr) {                                                                                      \
        return;                                                                                                        \
    }

namespace godot {
    class TrainPart : public Node {
            GDCLASS(TrainPart, Node)
        public:
            static void _bind_methods();

        private:
            Dictionary state;
            bool _commands_registered = false;

        protected:
            // This method cannot be marked as override because it's not virtual in the base class (Wrapped).
            // It is, however, used by GDCLASS macro to register notification callback.
            void _notification(int p_what); // NOLINT(bugprone-derived-method-shadowing-base-method)
            bool enabled = true;
            bool enabled_changed = false;
            bool dirty = false;
            TrainController *train_controller_node;

            /* If there is a need to separate the mover initialization stages from its update,
             * this method should be implemented similarly to _do_update_internal_mover(),
             * and should be called by TrainPart::initialize_mover() */
            // virtual void _do_initialize_internal_mover(TMoverParameters *mover) = 0;

            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            /* Transfers data from Godot's node to original/internal Mover instance.
             * `mover` is always set */

            virtual void _do_update_internal_mover(TMoverParameters *p_mover);

            /* Transfers state from the original/internal Mover instance to Godot's Dictionary.
             * `mover` and `state` are always set
             * */

            virtual void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config);

            virtual void _do_process_mover(TMoverParameters *p_mover, double p_delta);

            virtual void _register_commands();
            virtual void _unregister_commands();

            TMoverParameters *get_mover();

        public:
            // Physics-tick-only lifecycle, driven exclusively by TrainPhysicsServer::step_physics()
            // (never by Node's own _process()/_physics_process()): flush dirty state/enabled-change
            // bookkeeping, then compute, then sync Godot-visible state.
            void _flush_dirty_state();
            virtual void _process_mover_thread_safe(double p_delta);
            virtual void _post_process_mover_sync();

            void register_command(const String &p_command, const Callable &p_callback);
            void unregister_command(const String &p_command, const Callable &p_callback);
            void
            send_command(const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            void broadcast_command(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            void log(GameLog::LogLevel p_level, const String &p_line);
            void log_debug(const String &p_line);
            void log_info(const String &p_line);
            void log_warning(const String &p_line);
            void log_error(const String &p_line);

            void set_enabled(bool p_value);
            bool get_enabled();

            /* If there is a need to separate the mover initialization stages from its update,
             * this method should be implemented similarly to update_mover(),
             * and should be called from TrainController::initialize_mover() */
            // void initialize_mover(TrainController *train_controller_node);

            /* High level method for updating the state of the Mover */
            void update_mover();

            /* High level method for getting the state of the Mover */
            Dictionary get_mover_state();
            void emit_config_changed_signal();
    };
} // namespace godot
