#pragma once
#include <godot_cpp/classes/node.hpp>
#include "maszyna/McZapkie/MOVER.h"


namespace godot {
    class TrainBrake;
    class TrainPart;
    class TrainEngine;
    class TrainSwitch;
    class TrainSecuritySystem;

    class TrainController : public Node {
            GDCLASS(TrainController, Node)

            TMoverParameters *mover;
            double initial_velocity = 0.0;
            int cabin_number = 0;
            String type_name = "";
            void initialize_mover();
            bool _dirty = false;      // Refreshes all elements
            bool _dirty_prop = false; // Refreshes only TrainController's properties
            Dictionary state;

            bool sw_battery_enabled = false;
            bool sw_fuel_pump_enabled = false;
            bool sw_oil_pump_enabled = false;

            double nominal_battery_voltage = 0.0; // FIXME: move to TrainPower ?
            double mass = 0.0;

            static void _collect_train_switches(const Node *node, Vector<TrainSwitch *> &train_switches);
            void _connect_signals_to_train_part(TrainPart *part);

        protected:
            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            // TrainController mozna bedzie rozszerzac klasami pochodnymi i przeslaniac metody
            void _do_update_internal_mover(TMoverParameters *mover) const;
            static void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state);

        public:
            void _process(double delta) override;
            void _ready() override;
            Dictionary get_mover_state();
            void update_mover() const;
            void _on_train_part_config_changed(TrainPart *part) const;

            TMoverParameters *get_mover() const;
            static void _bind_methods();

            NodePath engine_path;
            mutable TrainEngine *_engine;
            void set_engine_path(const NodePath &p_path);
            NodePath get_engine_path() const;
            TrainEngine *get_engine() const;

            NodePath brake_path;
            mutable TrainBrake *_brake;
            void set_brake_path(const NodePath &p_path);
            NodePath get_brake_path() const;
            TrainBrake *get_brake() const;

            NodePath security_system_path;
            mutable TrainSecuritySystem *_security_system;
            void set_security_system_path(const NodePath &p_path);
            NodePath get_security_system_path() const;
            TrainSecuritySystem *get_security_system() const;

            String get_type_name() const;
            void set_type_name(const String &type_name);
            void set_nominal_battery_voltage(double p_nominal_battery_voltage);
            double get_nominal_battery_voltage() const;
            static void set_brake_level(double p_brake_level);
            void set_battery_enabled(bool p_battery_enabled);
            bool get_battery_enabled() const;
            void set_mass(double p_mass);
            double get_mass() const;

            // FIXME: move to TrainDieselEngine
            bool get_fuel_pump_enabled() const;
            void set_fuel_pump_enabled(bool p_state);
            bool get_oil_pump_enabled() const;
            void set_oil_pump_enabled(bool p_state);

            Vector<TrainSwitch *> get_train_switches() const;

            TrainController();
            ~TrainController() override = default;
    };

} // namespace godot
