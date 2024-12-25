#include <godot_cpp/classes/gd_extension.hpp>
#include "TrainHeatingSystem.hpp"

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

    [[maybe_unused]] void TrainHeatingSystem::_do_update_internal_mover(TMoverParameters *mover) {
        ASSERT_MOVER(mover)
        power_source->update_state(mover->HeatingPowerSource);
        alternative_power_source->update_state(mover->AlterHeatPowerSource);
    }

    void TrainHeatingSystem::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        ASSERT_MOVER(mover);
        power_source->fetch_state(mover->HeatingPowerSource, state, "heating");
        alternative_power_source->fetch_state(mover->AlterHeatPowerSource, state, "alternative_heating");
    }

    void TrainHeatingSystem::on_power_source_change() {
        _dirty = true;
    }

    void TrainHeatingSystem::_enter_tree() {
        TrainPart::_enter_tree();
        if (!power_source.is_valid()) {
            auto *const ps = memnew(NotDefinedPowerSource());
            power_source = Ref{ps};
        }
        if (!alternative_power_source.is_valid()) {
            auto *const ps = memnew(NotDefinedPowerSource());
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
