#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/node.hpp>


namespace godot {
    class TrainBrake;
    class TrainPart;
    class TrainEngine;
    class TrainSecuritySystem;
    class TrainSystem;


    class TrainController : public Node {
            GDCLASS(TrainController, Node)
        private:
            TMoverParameters *mover{};
            String train_id = "";

            double initial_velocity = 0.0;
            int cabin_number = 0;
            String type_name = "";
            void initialize_mover();
            bool _dirty = false;      // Refreshes all elements
            bool _dirty_prop = false; // Refreshes only TrainController's properties
            bool occupied = true;
            Dictionary state;
            Dictionary config;
            Dictionary internal_state;

            double battery_voltage = 0.0; // FIXME: move to TrainPower ?
            double mass = 0.0;
            double power = 0.0;
            double max_velocity = 0.0;
            int radio_channel = 0;
            int radio_channel_min = 0;
            int radio_channel_max = 10;

            bool prev_is_powered = false;
            bool prev_radio_enabled = false;
            int prev_radio_channel = radio_channel;

            bool state_changed = false;

            String axle_arrangement = "";

            void _collect_train_parts(const Node *node, Vector<TrainPart *> &train_parts) {};
            void _on_state_changed();

        private:
            void _update_mover_config_if_dirty();
            void _handle_mover_update();

        protected:
            /* _do_initialize_internal_mover() and _do_fetch_state_from_mover() are part of an internal interface
             * for creating Train nodes. Pointer to `mover` and reference to `state` should stay "as is",
             * because the mover initialization and state sharing routines can be changed in the future. */

            Dictionary get_mover_state();
            // TrainController mozna bedzie rozszerzac klasami pochodnymi i przeslaniac metody
            void _do_update_internal_mover(TMoverParameters *mover) const;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) const;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state);
            void _process_mover(double delta);
            void _notification(int p_what);


        public:
            static const char *MOVER_CONFIG_CHANGED_SIGNAL;
            static const char *MOVER_INITIALIZED_SIGNAL;
            static const char *STATE_CHANGED;
            static const char *POWER_CHANGED_SIGNAL;
            static const char *COMMAND_RECEIVED;
            static const char *RADIO_TOGGLED;
            static const char *RADIO_CHANNEL_CHANGED;
            static const char *CONFIG_CHANGED;

            Dictionary get_config() const;
            void update_config(const Dictionary &p_config);
            void _process(double delta) override;
            void send_command(const StringName &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void battery(bool p_enabled);
            void main_controller_increase(int p_step = 1);
            void main_controller_decrease(int p_step = 1);
            void direction_increase();
            void direction_decrease();
            void radio(bool p_enabled);
            void radio_channel_set(int p_channel);
            void radio_channel_increase(int step = 1);
            void radio_channel_decrease(int step = 1);
            void emit_command_received_signal(
                    const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void broadcast_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            void register_command(const String &command, const Callable &callable);
            void unregister_command(const String &command, const Callable &callable);
            void update_mover();
            void mark_state_changed();

            TMoverParameters *get_mover() const;
            static void _bind_methods();
            String get_train_id() const;
            void set_train_id(const String &train_id);
            String get_type_name() const;
            void set_type_name(const String &type_name);
            void set_battery_voltage(double p_value);
            double get_battery_voltage() const;
            void set_mass(double p_mass);
            double get_mass() const;
            void set_power(double p_power);
            double get_power() const;
            void set_max_velocity(double p_value);
            double get_max_velocity() const;
            void set_axle_arrangement(const String &p_value);
            int get_radio_channel_min() const;
            void set_radio_channel_min(int p_value);
            int get_radio_channel_max() const;
            void set_radio_channel_max(int p_value);
            String get_axle_arrangement() const;
            void set_occupied(const bool p_occupied);
            bool get_occupied() const;

            void set_state(const Dictionary &p_state);
            Dictionary get_state();
    };

} // namespace godot
