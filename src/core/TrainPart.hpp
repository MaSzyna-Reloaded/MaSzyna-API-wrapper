#pragma once
#include <functional>
#include <godot_cpp/classes/node.hpp>
#include "./TrainSystem.hpp"
#include "TrainController.hpp"

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
            void _notification(int p_what);
            bool enabled = true;
            bool enabled_changed = false;
            bool _dirty = false;
            TrainController *train_controller_node;

            /* Jesli bedzie potrzeba rozdzielenia etapow inicjalizacji movera od jego aktualizacji,
             * to ta metoda powinna byc zaimplementowana analogicznie do _do_update_internal_mover(),
             * i powinna byc wywolywana przez TrainPart::initialize_mover() */
            // virtual void _do_initialize_internal_mover(TMoverParameters *mover) = 0;

            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            /* Transfers data from Godot's node to original/internal Mover instance.
             * `mover` is always set */

            virtual void _do_update_internal_mover(TMoverParameters *mover);

            /* Transfers state from the original/internal Mover instance to Godot's Dictionary.
             * `mover` and `state` are always set
             * */

            virtual void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) = 0;
            virtual void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config);

            virtual void _do_process_mover(TMoverParameters *mover, double delta);

            virtual void _register_commands();
            virtual void _unregister_commands();

            TMoverParameters *get_mover();

        public:
            void _process(double delta) override;
            virtual void _process_mover(double delta);

            void register_command(const String &command, const Callable &callback);
            void unregister_command(const String &command, const Callable &callback);
            void send_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void broadcast_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void log(const TrainSystem::TrainLogLevel level, const String &line);
            void log_debug(const String &line);
            void log_info(const String &line);
            void log_warning(const String &line);
            void log_error(const String &line);

            void set_enabled(bool p_value);
            bool get_enabled();

            /* Jesli bedzie potrzeba rozdzielenia etapow inicjalizacji movera od jego aktualizacji,
             * to ta metoda powinna byc zaimplementowana analogicznie do update_mover(),
             * i powinna byc wywolywana z poziomu TrainController::initialize_mover() */
            // void initialize_mover(TrainController *train_controller_node);

            /* High level method for updating the state of the Mover */
            void update_mover();

            /* High level method for getting the state of the Mover */
            Dictionary get_mover_state();
            TrainPart();
            ~TrainPart() override = default;
            void emit_config_changed_signal();
    };
} // namespace godot
