#pragma once
#include "../core/TrainPart.hpp"
#include "macros.hpp"

namespace godot {
    class TrainController;
    class TrainBuffCoupl: public TrainPart {
            GDCLASS(TrainBuffCoupl, TrainPart);
        private:
            static void _bind_methods();
        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            enum CouplerType {
                COUPLER_TYPE_AUTOMATIC,
                COUPLER_TYPE_SCREW,
                COUPLER_TYPE_CHAIN,
                COUPLER_TYPE_BARE,
                COUPLER_TYPE_ARTICULATED
            };

            // Bit-flag enums for coupler features and power transfer
            enum AllowedFlagBits {
                ALLOWED_MECHANICAL = 1,
                ALLOWED_BRAKE_PIPE = 2,
                ALLOWED_MULTIPLE_CONTROL = 4,
                ALLOWED_HIGH_VOLTAGE = 8,
                ALLOWED_PASSAGE = 16,
                ALLOWED_AIR_8_BAR = 32,
                ALLOWED_HEATING = 64,
                ALLOWED_FIXED_COUPLING_LOCK = 128,
                // The following are only valid together with fixed coupling lock, but included for completeness
                ALLOWED_ELEC_24V = 256,
                ALLOWED_ELEC_110V = 512,
                ALLOWED_ELEC_3x400V = 1024
            };

            enum PowerFlagBits {
                POWER_24V = 256,
                POWER_110V = 512,
                POWER_3x400V = 1024
            };

            enum BufferLocation {
                BUFFER_LOCATION_FRONT,
                BUFFER_LOCATION_BACK,
                BUFFER_LOCATION_BOTH
            };

            MAKE_MEMBER_GS(double, buffer_stiffness_k, 1.0);
            MAKE_MEMBER_GS(double, buffer_max_compression_tolerance, 0.1);
            MAKE_MEMBER_GS(double, buffer_max_tension_tolerance, 1000.0);
            MAKE_MEMBER_GS(double, coupler_stiffness_k, 1.0);
            MAKE_MEMBER_GS(double, coupler_max_compression_tolerance, 0.1);
            MAKE_MEMBER_GS(double, coupler_max_tension_tolerance, 1000.0);
            MAKE_MEMBER_GS(double, damping_beta, 0.0);
            MAKE_MEMBER_GS(int, allowed_flag, 0);
            MAKE_MEMBER_GS(int, automatic_flag, 0);
            MAKE_MEMBER_GS(int, power_flag, 0);
            MAKE_MEMBER_GS(int, power_coupling, 128);
            MAKE_MEMBER_GS(String, control_type, "");
            MAKE_MEMBER_GS_NR(CouplerType, coupler_type, CouplerType::COUPLER_TYPE_AUTOMATIC);
            MAKE_MEMBER_GS_NR(BufferLocation, buffer_location, BufferLocation::BUFFER_LOCATION_FRONT);

            void couple();
            void decouple();
    };
} //namespace godot

VARIANT_ENUM_CAST(TrainBuffCoupl::CouplerType)
VARIANT_ENUM_CAST(TrainBuffCoupl::AllowedFlagBits)
VARIANT_ENUM_CAST(TrainBuffCoupl::PowerFlagBits)
VARIANT_ENUM_CAST(TrainBuffCoupl::BufferLocation)
