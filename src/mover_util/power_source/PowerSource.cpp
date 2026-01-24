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

    void PowerSource::update_config(TPowerParameters &p_power_parameters, TMoverParameters &p_mover) const {
        p_power_parameters.MaxCurrent = max_current;
        p_power_parameters.MaxVoltage = max_voltage;
        p_power_parameters.IntR = int_r;
    }

    void PowerSource::fetch_config(
            const TPowerParameters &p_power_parameters, godot::Dictionary &p_config,
            const godot::String &p_prefix) const {
        p_config[p_prefix + godot::String("/source_type")] = get_power_source_name(p_power_parameters.SourceType);
        p_config[p_prefix + godot::String("/max_voltage")] = p_power_parameters.MaxVoltage;
        p_config[p_prefix + godot::String("/max_current")] = p_power_parameters.MaxCurrent;
        p_config[p_prefix + godot::String("/int_r")] = p_power_parameters.IntR;
    }

    void PowerSource::fetch_state(
            const TPowerParameters &p_power_parameters, godot::Dictionary &p_state,
            const godot::String &p_prefix) const {
        // Base PowerSource has no dynamic state
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

    PowerType i_cast(TPowerType p) {
        switch (p) {
            case TPowerType::NoPower:
                return PowerType::NoPower;
            case TPowerType::BioPower:
                return PowerType::BioPower;
            case TPowerType::MechPower:
                return PowerType::MechPower;
            case TPowerType::ElectricPower:
                return PowerType::ElectricPower;
            case TPowerType::SteamPower:
                return PowerType::SteamPower;
            default:
                return PowerType::NoPower;
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

    String PowerSource::to_string(TPowerSource p) {
        switch (p) {
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
    }

    PowerType PowerSource::from_string(const godot::String &p) {
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
} // namespace godot
