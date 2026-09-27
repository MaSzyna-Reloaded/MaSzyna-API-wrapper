#pragma once
#include "VehicleController.hpp"

namespace godot {
    /// A railway vehicle: what VehicleController says of every vehicle, plus the railway's own -
    /// its cabs, couplers and consist, the master controller and reverser, the relays, the low
    /// voltage, the battery and the converter, and the stepping RailVehicleServer runs it by along
    /// its track. Railway components (RailVehicleComponent) belong to one of these.
    class RailVehicleController : public VehicleController {
            GDCLASS(RailVehicleController, VehicleController)

        private:
            bool prev_is_powered = false;
            int prev_cabin_occupied = 0;

        protected:
            static void _bind_methods();
            void _register_commands() override;
            void _unregister_commands() override;
            void _fill_state_dictionary(Dictionary &p_state) const override;

        public:
            /* Live state, read straight from the backend - nothing is stored. Public because it
             * is: every one of these is bound for GDScript, and a reader in C++ - the node that
             * draws the vehicle, a component of another kind - has the same right to it as a
             * script has. */
            /* The battery as it actually is, which drains and recharges. The authored
             * `battery_voltage` property next to it is the nominal one the vehicle is built with
             * and that the simulation keeps as the nominal battery voltage - the two are only equal
             * on a full battery. */
            virtual double get_live_battery_voltage() const = 0;
            virtual double get_tachometer_speed() const = 0;
            virtual double get_tachometer_speed_jump() const = 0;
            virtual double get_tachometer_clock_speed() const = 0;
            virtual int get_direction_absolute() const = 0;
            virtual int get_cabin() const = 0;
            virtual bool get_cabin_controleable() const = 0;
            virtual int get_cabin_occupied() const = 0;
            virtual bool get_battery_enabled() const = 0;
            /* ConverterFlag: the converter runs */
            virtual bool get_converter_enabled() const = 0;
            /* ConverterAllow: the converter's switch is on */
            virtual bool get_converter_allowed() const = 0;
            /* ConverterStartDelayTimer [s] */
            virtual double get_converter_time_to_start() const = 0;
            /* Metres since the distance counter was started, or -1 while it is off
             * (TTrain::m_distancecounter, Train.h:904) */
            virtual double get_distance_counter() const = 0;
            virtual double get_power24_voltage() const = 0;
            virtual bool get_power24_available() const = 0;
            virtual bool get_power110_available() const = 0;
            virtual double get_current0() const = 0;
            virtual double get_current1() const = 0;
            virtual double get_current2() const = 0;
            virtual bool get_relay_novolt() const = 0;
            virtual bool get_relay_overvoltage() const = 0;
            virtual bool get_relay_ground() const = 0;
            virtual int get_train_damage() const = 0;
            virtual int get_controller_second_position() const = 0;
            virtual int get_controller_main_position() const = 0;
            virtual int get_controller_joint_position() const = 0;
            virtual int get_controller_main_actual_position() const = 0;
            /* DelayCtrlFlag: the master controller waits on its first position for the line
             * contactors (Mover.cpp) */
            virtual bool get_controller_main_delayed() const = 0;
            /* A coupler pulled past its strength (stretch_duration > 0, Mover.cpp:5405) */
            virtual bool get_coupler_stretched() const = 0;
            /* The last master controller position that gives no power (MainCtrlNoPowerPos(),
             * Mover.cpp:2694): 0, or the EIM controller's own */
            virtual int get_controller_main_no_power_position() const = 0;
            /* The Radio-Stop received and not yet acknowledged (RadioStopFlag) */
            virtual bool get_radio_stop_active() const = 0;
            virtual int get_circuit_rlist_size() const = 0;

            /* shared enum for every FIZ "...Start=" device activation mode field (Cntrl. section);
             * duplicated from RailVehicleEngine::StartMode to avoid a circular include (RailVehicleEngine.hpp includes
             * VehicleComponent.hpp, which includes this file) */
            enum StartMode {
                START_MODE_DISABLED,
                START_MODE_MANUAL,
                START_MODE_AUTOMATIC,
                START_MODE_MANUAL_WITH_AUTO_FALLBACK,
                START_MODE_CONVERTER,
                START_MODE_BATTERY,
                START_MODE_DIRECTION,
            };

            /* The element a coupler attached or detached, as the original names them (coupler,
             * brake hose, main hose, control, gangway, heating); permanent marks the couplings inside
             * one unit (coupling::permanent) */
            enum CouplingElement {
                COUPLING_ELEMENT_COUPLER,
                COUPLING_ELEMENT_BRAKEHOSE,
                COUPLING_ELEMENT_MAINHOSE,
                COUPLING_ELEMENT_CONTROL,
                COUPLING_ELEMENT_GANGWAY,
                COUPLING_ELEMENT_HEATING,
                COUPLING_ELEMENT_PERMANENT,
            };

            /* Type= : bitmask identifying a vehicle's special-cased behavior family */
            enum TrainType {
                TRAIN_TYPE_DEFAULT = 0,
                TRAIN_TYPE_EZT = 1,
                TRAIN_TYPE_ET41 = 2,
                TRAIN_TYPE_ET42 = 4,
                TRAIN_TYPE_PSEUDODIESEL = 8,
                TRAIN_TYPE_ET22 = 0x10,
                TRAIN_TYPE_SN61 = 0x20,
                TRAIN_TYPE_EP05 = 0x40,
                TRAIN_TYPE_ET40 = 0x80,
                TRAIN_TYPE_181 = 0x100,
                TRAIN_TYPE_DMU = 0x200,
            };

            enum TrainPowerSource {
                POWER_SOURCE_NOT_DEFINED,
                POWER_SOURCE_INTERNAL,
                POWER_SOURCE_TRANSDUCER,
                POWER_SOURCE_GENERATOR,
                POWER_SOURCE_ACCUMULATOR,
                POWER_SOURCE_CURRENTCOLLECTOR,
                POWER_SOURCE_POWERCABLE,
                POWER_SOURCE_HEATER,
                POWER_SOURCE_MAIN
            };

            enum TrainPowerType {
                POWER_TYPE_NONE,
                POWER_TYPE_BIO,
                POWER_TYPE_MECH,
                POWER_TYPE_ELECTRIC,
                POWER_TYPE_STEAM
            };

            static const char *power_changed_signal;
            static const char *cabin_occupied_changed;
            /// The consist this vehicle belongs to gained or lost a vehicle
            static const char *consist_changed_signal;
            /// One coupling element attached / detached, once per event. Two signals rather than
            /// one carrying a direction: every listener would have opened by branching on it.
            static const char *coupler_attached_signal;
            static const char *coupler_detached_signal;

            virtual void battery(bool p_enabled) const = 0;
            /* The converter switched (ConverterSwitch(), Mover.cpp:3702): the cab's own, sent along
             * the control line to the vehicles that carry one */
            virtual void converter(bool p_enabled) const = 0;
            virtual void cab_activation(bool p_enabled) const = 0;
            virtual void cab_activation_auto() const = 0;
            virtual void cab_change(int p_direction) const = 0;
            /* The main circuit's ground relay reset (maincircuitgroundreset, RelayReset(), Mover.cpp:6653) */
            virtual void ground_relay_reset() const = 0;
            /* The anti-slip brake pressed (antislip, AntiSlippingButton()) */
            virtual void antislip() const = 0;
            virtual void main_controller_increase(int p_step = 1) const = 0;
            virtual void main_controller_decrease(int p_step = 1) const = 0;
            virtual void second_controller_increase(int p_step = 1) const = 0;
            virtual void second_controller_decrease(int p_step = 1) const = 0;
            virtual void direction_increase() const = 0;
            virtual void direction_decrease() const = 0;
            /* distancecounter_sw: pressed starts the distance counter anew (Train.cpp:1552) */
            virtual void distance_counter_activate(bool p_pressed) = 0;
            virtual double process_movement(double p_delta) = 0;
            virtual void update_location() = 0;
            virtual void
            update_neighbour(int p_end, RailVehicleController *p_other, int p_other_end, double p_track_distance) = 0;
            virtual void compute_forces(double p_delta) = 0;
            virtual void compute_movement(double p_delta) = 0;
            virtual void compute_fast_movement(double p_delta) = 0;
            virtual void couple(RailVehicleController *p_other, int p_end, int p_other_end, int p_coupling_type) = 0;
            virtual void uncouple(int p_end) = 0;
            virtual bool is_coupled(int p_end) const = 0;
            /* Whether this end is joined by p_element (TestFlag(Couplers[end].CouplingFlag, ...)) */
            virtual bool is_coupled_by(int p_end, CouplingElement p_element) const = 0;
            virtual void coupler_connect(const Variant &p_where) = 0;
            virtual void coupler_disconnect(const Variant &p_where) = 0;
            virtual RailVehicleController *get_coupled_controller(int p_end) const = 0;
            virtual int get_coupled_end(int p_end) const = 0;
            void change_track(const String &p_track_name, float p_track_offset, int p_track_direction);
            void update_state() override;
            void initialize() override;

            MAKE_MEMBER_GS(double, battery_voltage, 0.0); // FIXME: move to TrainPower ?
            MAKE_MEMBER_GS_NR(TrainType, train_type, TRAIN_TYPE_DEFAULT);
            MAKE_MEMBER_GS(double, reduced_mass, 0.0);
            MAKE_MEMBER_GS(double, sand_capacity, 0.0);
            MAKE_MEMBER_GS(double, heating_power, 0.0);
            MAKE_MEMBER_GS(double, light_power, 0.0);

            /* Cntrl. (ogolne, bateria/przekaznik ziemnozwarciowy/oswietlenie przedzialow/aktywacja kabiny) */
            MAKE_MEMBER_GS_NR(StartMode, cntrl_battery_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS_NR(StartMode, cntrl_converter_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS(double, cntrl_converter_start_delay, 0.0);
            MAKE_MEMBER_GS_NR(StartMode, cntrl_ground_relay_start_mode, START_MODE_MANUAL);
            MAKE_MEMBER_GS_NR(StartMode, cntrl_compartment_lights_start_mode, START_MODE_DISABLED);
            MAKE_MEMBER_GS(bool, cntrl_automatic_cab_activation, true);
            MAKE_MEMBER_GS(int, cntrl_inactive_cab_flag, 0);
    };
} // namespace godot

VARIANT_ENUM_CAST(RailVehicleController::TrainPowerSource);
VARIANT_ENUM_CAST(RailVehicleController::TrainPowerType);
VARIANT_ENUM_CAST(RailVehicleController::CouplingElement);
VARIANT_ENUM_CAST(RailVehicleController::TrainType);
VARIANT_ENUM_CAST(RailVehicleController::StartMode);
