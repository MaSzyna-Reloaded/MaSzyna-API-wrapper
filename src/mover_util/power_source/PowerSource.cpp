#include "Mod.hpp"
#include "PowerSource.hpp"

namespace godot {

    void PowerSource::_bind_methods() {
        BIND_ENUM_CONSTANT(NoPower)
        BIND_ENUM_CONSTANT(BioPower)
        BIND_ENUM_CONSTANT(MechPower)
        BIND_ENUM_CONSTANT(ElectricPower)
        BIND_ENUM_CONSTANT(SteamPower)
        ADD_SIGNAL(MethodInfo(POWER_SOURCE_CHANGED));
    }

    void PowerSource::update_config(TPowerParameters &p_power_parameters) const {
        p_power_parameters.SourceType = get_source_type();
    }

    void PowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, godot::Dictionary &p_config,
            const godot::String &p_prefix) const {
        p_config[p_prefix + godot::String("/source_type")] = get_power_source_name(p_power_parameters.SourceType);
    }

    TPowerType PowerSource::cast(PowerType p) {
        switch (p) {
            case NoPower:
                return TPowerType::NoPower;
            case BioPower:
                return TPowerType::BioPower;
            case MechPower:
                return TPowerType::MechPower;
            case ElectricPower:
                return TPowerType::ElectricPower;
            case SteamPower:
                return TPowerType::SteamPower;
            default:
                return TPowerType::NoPower;
        }
    }

    PowerSource::PowerType i_cast(TPowerType p) {
        switch (p) {
            case TPowerType::NoPower:
                return PowerSource::NoPower;
            case TPowerType::BioPower:
                return PowerSource::BioPower;
            case TPowerType::MechPower:
                return PowerSource::MechPower;
            case TPowerType::ElectricPower:
                return PowerSource::ElectricPower;
            case TPowerType::SteamPower:
                return PowerSource::SteamPower;
            default:
                return PowerSource::NoPower;
        }
    }

    String PowerSource::to_string(PowerType p) {
        switch (p) {
            case NoPower:
                return "NoPower";
            case BioPower:
                return "BioPower";
            case MechPower:
                return "MechPower";
            case ElectricPower:
                return "ElectricPower";
            case SteamPower:
                return "SteamPower";
            default:
                return "NoPower";
        }
    }

    PowerSource::PowerType PowerSource::from_string(const godot::String &p) {
        if (p == "NoPower") {
            return NoPower;
        } else if (p == "BioPower") {
            return BioPower;
        } else if (p == "MechPower") {
            return MechPower;
        } else if (p == "ElectricPower") {
            return ElectricPower;
        } else if (p == "SteamPower") {
            return SteamPower;
        } else {
            return NoPower;
        }
    }

    String PowerSource::get_power_source_name(const TPowerSource s) {
        switch (s) {
            case TPowerSource::NotDefined:
                return "NotDefined";
            case TPowerSource::InternalSource:
                return "InternalSource";
            case TPowerSource::Transducer:
                return "Transducer";
            case TPowerSource::Generator:
                return "Generator";
            case TPowerSource::Accumulator:
                return "Accumulator";
            case TPowerSource::CurrentCollector:
                return "CurrentCollector";
            case TPowerSource::PowerCable:
                return "PowerCable";
            case TPowerSource::Heater:
                return "Heater";
            case TPowerSource::Main:
                return "Main";
            default:
                return "Unknown";
        }
    };


    Ref<PowerSource> PowerSource::create(const TPowerParameters &p_power_parameters) {
        PowerSource *ps = nullptr;

        switch (p_power_parameters.SourceType) {
            case TPowerSource::NotDefined:
                ps = memnew(NotDefinedPowerSource);
                static_cast<NotDefinedPowerSource *>(ps)->set_power_type(i_cast(p_power_parameters.PowerType));
                break;
            case TPowerSource::InternalSource:
                ps = memnew(InternalPowerSource);
                static_cast<InternalPowerSource *>(ps)->set_power_type(i_cast(p_power_parameters.PowerType));
                break;
            case TPowerSource::Transducer:
                ps = memnew(TransducerPowerSource);
                static_cast<TransducerPowerSource *>(ps)->set_input_voltage(p_power_parameters.Transducer.InputVoltage);
                break;
            case TPowerSource::Generator:
                // TBD
                break;
            case TPowerSource::Accumulator:
                // TBD
                break;
            case TPowerSource::CurrentCollector:
                // TBD
                break;
            case TPowerSource::PowerCable:
                // TBD
                break;
            case TPowerSource::Heater:
                // TBD
                break;
            case TPowerSource::Main:
                // TBD
                break;
        }
        return Ref<PowerSource>(ps);
    };
} // namespace godot
