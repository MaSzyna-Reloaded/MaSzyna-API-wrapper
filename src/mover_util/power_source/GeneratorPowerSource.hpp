#pragma once

#include "../../macros.hpp"
#include "PowerSource.hpp"
#include <godot_cpp/godot.hpp>

namespace godot {
    class GeneratorPowerSource : public PowerSource {
            GDCLASS(GeneratorPowerSource, PowerSource)

        public:
            enum GeneratorType : int{
                ElectricSeriesMotor,
                DieselEngine,
                SteamEngine,
                WheelsDriven,
                Dumb,
                DieselElectric,
                DumbDE,
                ElectricInductionMotor,
                Main,
                None
            };

            MAKE_MEMBER_GS_NR(GeneratorType, generator_type, GeneratorType::None);
            MAKE_MEMBER_GS(double, generator_min_voltage, 0.0);
            MAKE_MEMBER_GS(double, generator_max_voltage, 0.0);
            MAKE_MEMBER_GS(double, generator_min_rpm, 0.0);
            MAKE_MEMBER_GS(double, generator_max_rpm, 0.0);

        protected:
            static void _bind_methods();

        public:
            void update_config(TPowerParameters &params, TMoverParameters &mover) const override;
            void fetch_config(const TPowerParameters &params, Dictionary &state, const String &prefix) const override;
            void fetch_state(const TPowerParameters &params, Dictionary &state, const String &prefix) const override;
    };
} // namespace godot

VARIANT_ENUM_CAST(GeneratorPowerSource::GeneratorType);
