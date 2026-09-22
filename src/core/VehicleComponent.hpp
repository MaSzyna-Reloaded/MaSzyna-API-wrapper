#pragma once
#include "./GameLog.hpp"
#include "./TrainSystem.hpp"
#include "VehicleController.hpp"
#include <functional>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/vector.hpp>

#define ASSERT_MOVER(mover_ptr)                                                                                        \
    if ((mover_ptr) == nullptr) {                                                                                      \
        return;                                                                                                        \
    }

namespace godot {
    class VehicleComponent : public Node {
            GDCLASS(VehicleComponent, Node)
        public:
            static void _bind_methods();

        private:
            bool _commands_registered = false;

        protected:
            // This method cannot be marked as override because it's not virtual in the base class (Wrapped).
            // It is, however, used by GDCLASS macro to register notification callback.
            void _notification(int p_what); // NOLINT(bugprone-derived-method-shadowing-base-method)
            bool enabled = true;
            bool enabled_changed = false;
            bool dirty = false;
            VehicleController *train_controller_node;

            /* Jesli bedzie potrzeba rozdzielenia etapow inicjalizacji movera od jego aktualizacji,
             * to ta metoda powinna byc zaimplementowana analogicznie do _do_update_internal_mover(),
             * i powinna byc wywolywana przez VehicleComponent::initialize_mover() */
            // virtual void _do_initialize_internal_mover(TMoverParameters *mover) = 0;


            /* Transfers data from Godot's node to original/internal Mover instance.
             * `mover` is always set */

            virtual void _do_update_internal_mover(TMoverParameters *p_mover);



            virtual void _do_process_mover(TMoverParameters *p_mover, double p_delta);

            virtual void _register_commands();
            virtual void _unregister_commands();

            TMoverParameters *get_mover() const;

        public:
            void _process(double p_delta) override;
            virtual void _process_mover(double p_delta);

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

            /* Jesli bedzie potrzeba rozdzielenia etapow inicjalizacji movera od jego aktualizacji,
             * to ta metoda powinna byc zaimplementowana analogicznie do apply_config(),
             * i powinna byc wywolywana z poziomu VehicleController::initialize_mover() */
            // void initialize_mover(VehicleController *train_controller_node);

            /* Applies this part's authored configuration to the vehicle and publishes back
             * whatever config the vehicle derives from it. The simulation backend is an
             * implementation detail: the interface knows state and config, nothing else. */
            void apply_config();

            /* Writes this component's share of the vehicle dump. Called only when somebody asks
             * for a dump - a console, a test, a diagnostic - never per frame: the live values are
             * read from this component's own typed properties.
             *
             * Keys carry the component's name (`heating_enabled`, not `enabled`) because the dump
             * is one flat Dictionary for the whole vehicle. A key the vehicle's variant does not
             * have is simply not written, so has() keeps meaning what it meant. */
            virtual void _fill_state_dictionary(Dictionary &p_state) const;
            Dictionary get_state();

            /* This component's share of the vehicle's configuration dump. Unlike the state, the
             * configuration is the wrapper's own - its properties and enums are the authoring
             * source of truth and the backend is configured from them. */
            virtual void _fill_config_dictionary(Dictionary &p_config) const;
            Dictionary get_config();
            void emit_config_changed_signal();
            void mark_dirty();
    };
} // namespace godot
