#include "TrainHeatingSystem.hpp"
#include <godot_cpp/godot.hpp>

#include "../../mover_util/power_source/NotDefinedPowerSource.hpp"

namespace godot {
    void TrainHeatingSystem::_bind_methods() {
        BIND_PROPERTY(
                Variant::OBJECT, "power_source", "power_source", &TrainHeatingSystem::set_power_source,
                &TrainHeatingSystem::get_power_source, "power_source");
        BIND_PROPERTY(
                Variant::OBJECT, "alternative_power_source", "alternative_power_source",
                &TrainHeatingSystem::set_alternative_power_source, &TrainHeatingSystem::get_alternative_power_source,
                "power_source");
        }

    void TrainHeatingSystem::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover)
        power_source->update_config(p_mover->HeatingPowerSource, *p_mover);
        alternative_power_source->update_config(p_mover->AlterHeatPowerSource, *p_mover);
    }

    void TrainHeatingSystem::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        ASSERT_MOVER(p_mover);
        reload_power_source(power_source, p_mover->HeatingPowerSource);
        reload_power_source(alternative_power_source, p_mover->AlterHeatPowerSource);
        power_source->fetch_config(p_mover->HeatingPowerSource, p_config, "heating");
        alternative_power_source->fetch_config(p_mover->AlterHeatPowerSource, p_config, "alternative_heating");
    }


    void TrainHeatingSystem::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        ASSERT_MOVER(p_mover);
        power_source->fetch_state(p_mover->HeatingPowerSource, p_config, "heating");
        alternative_power_source->fetch_state(p_mover->AlterHeatPowerSource, p_config, "alternative_heating");

        p_config["is_heating_allowed"] = p_mover->HeatingAllow;
        p_config["is_heating_active"] = p_mover->Heating;
        p_config["total_current_heating"] = p_mover->TotalCurrent;
        p_config["heating_power"] = p_mover->HeatingPower;
    }


    void TrainHeatingSystem::reload_power_source(Ref<PowerSource> &ref, TPowerParameters &src) {
        // Note: that changing `ref` is needed, because TMoverParameters::CheckLocomotiveParameters method may change
        // configuration after PowerSource object was created
        // Note 2: in legacy implementation, only HeatingSystem (not alternative) is changed, but this method is used
        // for both sources to future-proof future changes in original MaSzyna code

        // TODO: Rethink this implementation

        // if (src.SourceType != ref->get_source_type()) {
        //     ref->disconnect(PowerSource::POWER_SOURCE_CHANGED, Callable(this, "on_power_source_change"));
        //     ref = PowerSource::create(src);
        // }
    }

    void TrainHeatingSystem::on_power_source_change() {
        _dirty = true;
    }

    void TrainHeatingSystem::_enter_tree() {
        TrainPart::_enter_tree();
        if (!power_source.is_valid()) {
            auto *const ps = memnew(NotDefinedPowerSource);
            power_source = Ref{ps};
        }
        if (!alternative_power_source.is_valid()) {
            auto *const ps = memnew(NotDefinedPowerSource);
            alternative_power_source = Ref{ps};
        }
    }

    void TrainHeatingSystem::_ready() {
        TrainPart::_ready();
        power_source->connect(PowerSource::POWER_SOURCE_CHANGED, Callable(this, "on_power_source_change"));
        alternative_power_source->connect(PowerSource::POWER_SOURCE_CHANGED, Callable(this, "on_power_source_change"));
    }
} // namespace godot
