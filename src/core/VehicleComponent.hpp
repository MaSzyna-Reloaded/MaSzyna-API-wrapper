#pragma once
#include "./GameLog.hpp"
#include "./TrainSystem.hpp"
#include "VehicleComponentType.hpp"
#include "VehicleController.hpp"
#include <functional>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {
    /* One thing a vehicle is made of.
     *
     * Not a Node: a component belongs to a vehicle, not to a scene, and the vehicle creates it,
     * owns it and ticks it. What stands in the scene for the editor is a thin proxy. */
    class VehicleComponent : public Object {
            GDCLASS(VehicleComponent, Object)
        public:
            static void _bind_methods();

        private:
            bool _commands_registered = false;
            StringName component_tag;

        protected:
            bool enabled = true;
            bool enabled_changed = false;
            bool dirty = false;
            /* The vehicle this component belongs to, set when it joins it. A raw pointer with no
             * initialiser used to be safe only because nothing read this component before it
             * joined; a property read does (see FINDINGS.md, 2026-09-22). */
            VehicleController *train_controller_node = nullptr;

            /* Writes this component's configuration into whatever simulates the vehicle. What
             * that is belongs to the implementation - the interface does not name it. */
            virtual void _apply_configuration();

            /* One tick of this component. */
            virtual void _do_process_component(double p_delta);

            virtual void _register_commands();
            virtual void _unregister_commands();

            /* Whether the vehicle this component belongs to is simulated yet - a dump of a
             * component whose vehicle is not publishes nothing, rather than zeroes. */
            bool is_simulation_ready() const;

        public:

            /* Which kind this component is. Every interface answers for itself; an
             * implementation inherits the answer. */
            virtual VehicleComponentType::Type get_component_type() const;

            /* The vehicle takes the component in hand: from here on it owns it, ticks it and
             * frees it. Both are for the vehicle to call, not for a caller outside it. */
            void attach(VehicleController *p_controller);
            void detach();
            /* One tick of this component, driven by the vehicle that owns it. */
            void process(double p_delta);

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

            /* Applies this part's authored configuration to the vehicle and publishes back
             * whatever config the vehicle derives from it. The simulation backend is an
             * implementation detail: the interface knows state and config, nothing else. */
            void apply_config();

            /* Writes this component's share of the vehicle dump. Called only when somebody asks
             * for a dump - a console, a test, a diagnostic - never per frame: the live values are
             * read from this component's own typed properties.
             *
             * The dump is one flat Dictionary for the whole vehicle, so every key is qualified by
             * what owns it and no key is ever bare: `heating_enabled`, not `enabled`. A component
             * that publishes a whole sub-device's block qualifies it with a namespace instead -
             * `spring_brake/cylinder_pressure`, `current_collector/max_voltage`. A key is the
             * value's name and never the name of the accessor that produced it (see `FINDINGS.md`,
             * 2026-09-23, where `get_bogie_pivot_spacing()` as a key cost every vehicle its bogie
             * spacing). A key the vehicle's variant does not have is simply not written, so has()
             * keeps meaning what it meant. */
            virtual void _fill_state_dictionary(Dictionary &p_state) const;
            /* A scripted component's own kind, chosen by whoever wrote it. Empty on the
             * built-in ones, which are found by their ComponentType instead. */
            void set_component_tag(const StringName &p_tag);
            StringName get_component_tag() const;

            /* The vehicle this component belongs to */
            VehicleController *get_controller() const;

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

