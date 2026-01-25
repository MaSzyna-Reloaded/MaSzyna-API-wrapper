#pragma once

#include "../../macros.hpp"
#include "PowerSource.hpp"
#include "PowerTypeEnum.hpp"
#include <godot_cpp/godot.hpp>

namespace godot {
    class InternalPowerSource : public PowerSource {
            GDCLASS(InternalPowerSource, PowerSource)
        public:
            MAKE_MEMBER_GS_NR(PowerType, power_type, PowerType::NoPower);

        protected:
            static void _bind_methods();

        public:
            virtual TPowerSource get_source_type() const override {
                return TPowerSource::InternalSource;
            }
            virtual void update_config(TPowerParameters &params, TMoverParameters &mover) const override;
            virtual void
            fetch_config(const TPowerParameters &params, Dictionary &state, const String &prefix) const override;
            virtual void
            fetch_state(const TPowerParameters &params, Dictionary &state, const String &prefix) const override;
    };
} // namespace godot
