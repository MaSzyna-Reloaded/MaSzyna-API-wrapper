#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"

namespace godot {
    class VehicleController;
    class VehicleBuffCoupl : public VehicleComponent {
            GDCLASS(VehicleBuffCoupl, VehicleComponent);


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_BUFFERS;
            }

        private:
            static void _bind_methods();
        protected:
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
                ALLOWED_ELEC_3X400_V = 1024
            };
            enum PowerFlagBits { POWER_24V = 256, POWER_110V = 512, POWER_3X400_V = 1024 };
            enum BufferLocation { BUFFER_LOCATION_FRONT, BUFFER_LOCATION_BACK, BUFFER_LOCATION_BOTH };
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
            /// Which end of a vehicle a query is about.
            enum End {
                END_FRONT = 0,
                END_REAR = 1,
            };
            /* What is attached at an end. The coupler owns the coupling, so it answers for it -
             * a consumer picking a submodel to show has no business reading the simulation. */
            virtual bool is_coupled(End p_end) const = 0;
            virtual bool is_brake_hose_connected(End p_end) const = 0;
            virtual bool is_main_hose_connected(End p_end) const = 0;
            /// Which of the two coupled vehicles draws the coupler itself.
            virtual bool is_coupling_owner(End p_end) const = 0;
            /// The end of the neighbour this end is attached to.
            virtual End get_connected_end(End p_end) const = 0;

            virtual void couple() = 0;
            virtual void decouple() = 0;
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleBuffCoupl::CouplerType)
VARIANT_ENUM_CAST(VehicleBuffCoupl::AllowedFlagBits)
VARIANT_ENUM_CAST(VehicleBuffCoupl::PowerFlagBits)
VARIANT_ENUM_CAST(VehicleBuffCoupl::BufferLocation)
VARIANT_ENUM_CAST(VehicleBuffCoupl::End)
