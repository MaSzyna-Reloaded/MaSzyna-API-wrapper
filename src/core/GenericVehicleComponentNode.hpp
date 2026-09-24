#pragma once
#include "GenericVehicleComponent.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    /* Where a scripted vehicle component is authored.
     *
     * A component is not a node - it belongs to a vehicle, not to a scene - but a modder needs
     * somewhere to put one and something to attach a script to. This is that place: put it under
     * a VehiclePhysicsNode, write `extends GenericVehicleComponentNode`, and the vehicle you sit
     * under gets a component that calls your script.
     *
     * The methods a script overrides are the ones GenericVehicleComponent declares:
     * _process_component(delta), _get_component_state(), _get_component_config(). */
    class GenericVehicleComponentNode : public Node {
            GDCLASS(GenericVehicleComponentNode, Node)

        private:
            GenericVehicleComponent *component = nullptr;

        protected:
            static void _bind_methods();
            void _notification(int p_what); // NOLINT(bugprone-derived-method-shadowing-base-method)

        public:
            /* The component this node put into the vehicle, for anything that wants it directly */
            GenericVehicleComponent *get_component() const;

            /* What a script written on this node reaches for. They belong to the component; the
             * node forwards them so a modder's script reads the way it always did. */
            void register_command(const String &p_command, const Callable &p_callback);
            void unregister_command(const String &p_command, const Callable &p_callback);
            Variant send_command(const String &p_command, const Variant &p_p1, const Variant &p_p2);
            Dictionary get_vehicle_state();
            /* The vehicle this component belongs to. A modder's script reads what it needs off it
             * and off its other components, typed, the way the wrapper's own components do - the
             * whole-vehicle dump above is for a script that wants a handful of unrelated values. */
            VehicleController *get_controller() const;
            void log_debug(const String &p_line);
            void log_info(const String &p_line);
            void log_warning(const String &p_line);
            void log_error(const String &p_line);
    };
} // namespace godot
