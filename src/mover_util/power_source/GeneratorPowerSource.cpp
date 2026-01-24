#include "GeneratorPowerSource.hpp"

namespace godot {
    void GeneratorPowerSource::_bind_methods() {
        BIND_PROPERTY_W_HINT(Variant::INT,"type","type",&GeneratorPowerSource::get_generator_type,&GeneratorPowerSource::set_generator_type,"generator_type",PROPERTY_HINT_ENUM,"ElectricSeriesMotor,DieselEngine,SteamEngine,WheelsDriven,Dumb,DieselElectric,DumbDE,ElectricInductionMotor,Main,None")

        BIND_PROPERTY(Variant::FLOAT, "min_voltage", "voltage/min", &GeneratorPowerSource::get_generator_min_voltage, &GeneratorPowerSource::set_generator_min_voltage, "min_voltage");
        BIND_PROPERTY(Variant::FLOAT, "max_voltage", "voltage/max", &GeneratorPowerSource::get_generator_max_voltage, &GeneratorPowerSource::set_generator_max_voltage, "max_voltage");
        BIND_PROPERTY(Variant::FLOAT, "min_rpm", "rpm/min", &GeneratorPowerSource::get_generator_min_rpm, &GeneratorPowerSource::set_generator_min_rpm, "min_rpm");
        BIND_PROPERTY(Variant::FLOAT, "max_rpm", "rpm/max", &GeneratorPowerSource::get_generator_max_rpm, &GeneratorPowerSource::set_generator_max_rpm, "max_rpm");

        BIND_ENUM_CONSTANT(ElectricSeriesMotor);
        BIND_ENUM_CONSTANT(DieselEngine);
        BIND_ENUM_CONSTANT(SteamEngine);
        BIND_ENUM_CONSTANT(WheelsDriven);
        BIND_ENUM_CONSTANT(Dumb);
        BIND_ENUM_CONSTANT(DieselElectric);
        BIND_ENUM_CONSTANT(DumbDE);
        BIND_ENUM_CONSTANT(ElectricInductionMotor);
        BIND_ENUM_CONSTANT(Main);
        BIND_ENUM_CONSTANT(None);
    }

    void GeneratorPowerSource::update_config(TPowerParameters &params, TMoverParameters &mover) const {
        params.SourceType = TPowerSource::Generator;
        engine_generator &gen = params.EngineGenerator;
        if (generator_type == GeneratorType::Main) {
            gen.engine_revolutions = &mover.enrot;
        } else {
            // TODO: for engine types other than Main create requested engine object and link to its revolutions
            gen.engine_revolutions = nullptr;
            gen.revolutions = 0;
            gen.voltage = 0;
        }

        gen.voltage_min = generator_min_voltage / 60;
        gen.voltage_max = generator_max_voltage / 60;
        // NOTE: for consistency the fiz file specifies  revolutions per minute
        gen.revolutions_min = generator_min_rpm / 60;
        gen.revolutions_max = generator_max_rpm / 60;
    }

    void GeneratorPowerSource::fetch_config(const TPowerParameters &params, Dictionary &state, const String &prefix) const {
    }

    void GeneratorPowerSource::fetch_state(const TPowerParameters &params, Dictionary &state, const String &prefix) const {
        state[prefix + String("/engine_rpm")] = params.EngineGenerator.engine_revolutions;
        state[prefix + String("/engine_voltage")] = params.EngineGenerator.voltage;
    }
} // namespace godot
