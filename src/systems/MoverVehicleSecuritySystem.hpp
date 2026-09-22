#pragma once
#include "VehicleSecuritySystem.hpp"

namespace godot {
    /* VehicleSecuritySystem on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleSecuritySystem : public VehicleSecuritySystem {
            GDCLASS(MoverVehicleSecuritySystem, VehicleSecuritySystem);

        private:
            static void _bind_methods();
        private:
            friend class TSecuritySystem;
            bool previous_blinking = false;
            bool previous_beeping = false;
        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_beeping() const override;
            bool get_blinking() const override;
            bool get_radiostop_available() const override;
            bool get_vigilance_blinking() const override;
            bool get_cabsignal_blinking() const override;
            bool get_cabsignal_beeping() const override;
            bool get_braking() const override;
            bool get_engine_blocked() const override;
            bool get_separate_acknowledge() const override;
            void security_acknowledge(bool p_enabled) override;
            void security_cabsignal_acknowledge() override;
    };
} // namespace godot
