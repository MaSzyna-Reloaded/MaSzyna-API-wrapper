#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"

namespace godot {
    /* The vehicle's train radio - the public contract, with no backend in it. The channel and
     * the volume are the cab radio's own settings (TTrain's iRadioChannel and m_radiovolume in
     * the original, Train.h:918); whether the radio is on, powered, and what a Radio-Stop does
     * to the vehicle are the backend's to answer. */
    class VehicleRadio : public VehicleComponent {
            GDCLASS(VehicleRadio, VehicleComponent);

        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_RADIO;
            }

            static const char *radio_toggled_signal;
            static const char *channel_changed_signal;

            /* Original engine: Global.DefaultRadioVolume (Globals.h:181) */
            static constexpr double VOLUME_DEFAULT = 0.75;
            /* One press of radiovolumenext_sw:/radiovolumeprev_sw: (Train.cpp:8252) */
            static constexpr double VOLUME_STEP = 0.125;

        private:
            // the original starts at channel 1, not 0 (Driver.h: "int iRadioChannel = 1")
            int channel = 1;
            double volume = VOLUME_DEFAULT;

        protected:
            static void _bind_methods();
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            // the original hardcodes 1..10 for every vehicle (OnCommand_radiochannelset:
            // std::clamp((int)Command.param1, 1, 10)) - not configurable there, so this is only
            // the same range, never 0..0, which would clamp every channel change to nothing
            MAKE_MEMBER_GS(int, channel_min, 1);
            MAKE_MEMBER_GS(int, channel_max, 10);

            /* Live state */
            virtual bool get_enabled() const = 0;
            virtual bool get_powered() const = 0;
            int get_channel() const;
            double get_volume() const;

            virtual void radio(bool p_enabled) = 0;
            void channel_set(int p_channel);
            void channel_increase(int p_step = 1);
            void channel_decrease(int p_step = 1);
            void volume_increase(int p_step = 1);
            void volume_decrease(int p_step = 1);
            /* radiostop_sw: pressed sends Radio-Stop to every vehicle in range (Train.cpp:8149) */
            virtual void radio_stop(bool p_pressed) = 0;
            /* A Radio-Stop sent by a vehicle in range reached this one (TDynamicObject::RadioStop,
             * DynObj.cpp:7229) */
            virtual void radio_stop_receive() = 0;

            void _fill_state_dictionary(Dictionary &p_state) const override;
    };
} // namespace godot
