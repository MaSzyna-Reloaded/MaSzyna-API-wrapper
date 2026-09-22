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
            Dictionary state;
            bool _commands_registered = false;
            /* Local index -> the registry's global id, in declaration order */
            Vector<int> state_property_ids;

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

            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            /* Transfers data from Godot's node to original/internal Mover instance.
             * `mover` is always set */

            virtual void _do_update_internal_mover(TMoverParameters *p_mover);

            /* Transfers state from the original/internal Mover instance to Godot's Dictionary.
             * `mover` and `state` are always set
             * */

            /* The old push path: a component that has not been converted to declared
             * properties yet still merges its keys into the vehicle's Dictionary. Empty by
             * default, so a converted component simply stops having one. */
            virtual void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state);
            virtual void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config);

            virtual void _do_process_mover(TMoverParameters *p_mover, double p_delta);

            /* Declares what this component can be asked about, once, when it joins a vehicle.
             * Each declare_state_property() call returns the local index _get_state_property()
             * will be handed back. */
            virtual void _declare_state_properties();
            /* How many properties are declared so far - a subclass takes this after calling its
             * base's declaration, so its own local indices start where the base's end. */
            int get_state_property_count() const;

            int declare_state_property(
                    const StringName &p_name, Variant::Type p_type, const StringName &p_group = StringName());

            virtual void _register_commands();
            virtual void _unregister_commands();

            TMoverParameters *get_mover() const;

        public:
            /* The live value of one declared property, by the local index declare_state_property()
             * returned. Pure read: see CODE_STYLE.md, a getter never changes state. */
            virtual Variant _get_state_property(int p_local_index) const;

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

            /* This part's contribution to the vehicle state */
            Dictionary get_state();
            void emit_config_changed_signal();
            void mark_dirty();
    };
} // namespace godot
