#include "TrainHeatingSystem.hpp"
#include <godot_cpp/classes/gd_extension.hpp>

namespace godot {

    void TrainHeatingSystem::_bind_methods() {
        constexpr auto OBJ_FLAGS =
                // PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT | PROPERTY_USAGE_STORE_IF_NULL | PROPERTY_USAGE_DEFAULT;
                PROPERTY_USAGE_DEFAULT;

        ClassDB::bind_method(D_METHOD("set_power_source", "power_source"), &TrainHeatingSystem::set_power_source);
        ClassDB::bind_method(D_METHOD("get_power_source"), &TrainHeatingSystem::get_power_source);
        ClassDB::bind_method(
                D_METHOD("set_alternative_power_source", "alternative_power_source"),
                &TrainHeatingSystem::set_alternative_power_source);
        ClassDB::bind_method(
                D_METHOD("get_alternative_power_source"), &TrainHeatingSystem::get_alternative_power_source);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "power_source", PROPERTY_HINT_RESOURCE_TYPE, "PowerSource", OBJ_FLAGS),
                "set_power_source", "get_power_source");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "alternative_power_source", PROPERTY_HINT_RESOURCE_TYPE, "PowerSource",
                        OBJ_FLAGS),
                "set_alternative_power_source", "get_alternative_power_source");
    }

    void TrainHeatingSystem::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover)
        power_source->update_config(p_mover->HeatingPowerSource);
        alternative_power_source->update_config(p_mover->AlterHeatPowerSource);
    }

    void TrainHeatingSystem::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        ASSERT_MOVER(p_mover);
        reload_power_source(power_source, p_mover->HeatingPowerSource);
        reload_power_source(alternative_power_source, p_mover->AlterHeatPowerSource);
        power_source->fetch_config(p_mover->HeatingPowerSource, p_config, "heating");
        alternative_power_source->fetch_config(p_mover->AlterHeatPowerSource, p_config, "alternative_heating");
    }

    void TrainHeatingSystem::reload_power_source(Ref<PowerSource> &ref, TPowerParameters &src) {
        // Note: that changing `ref` is needed, because TMoverParameters::CheckLocomotiveParameters method may change
        // configuration after PowerSource object was created
        // Note 2: in legacy implementation, only HeatingSystem (not alternative) is changed, but this method is used
        // for both sources to future-proof future changes in original MaSzyna code
        if (src.SourceType != ref->get_source_type()) {
            ref->disconnect(PowerSource::POWER_SOURCE_CHANGED, Callable(this, "on_power_source_change"));
            ref = PowerSource::create(src);
        }
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

    // GETTERS AND SETTERS

    void TrainHeatingSystem::set_power_source(Ref<PowerSource> p_power_source) {
        power_source = p_power_source;
        _dirty = true;
    }

    Ref<PowerSource> TrainHeatingSystem::get_power_source() const {
        return power_source;
    }

    void TrainHeatingSystem::set_alternative_power_source(Ref<PowerSource> p_alternative_power_source) {
        alternative_power_source = p_alternative_power_source;
        _dirty = true;
    }

    Ref<PowerSource> TrainHeatingSystem::get_alternative_power_source() const {
        return alternative_power_source;
    }
} // namespace godot
