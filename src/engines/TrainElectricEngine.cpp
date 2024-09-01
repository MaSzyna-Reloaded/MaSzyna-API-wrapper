#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "TrainElectricEngine.hpp"

namespace godot {
    TrainElectricEngine::TrainElectricEngine() = default;

    void TrainElectricEngine::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_compressor_switch_pressed"), &TrainElectricEngine::set_compressor_switch_pressed);
        ClassDB::bind_method(
                D_METHOD("get_compressor_switch_pressed"), &TrainElectricEngine::get_compressor_switch_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/compressor"), "set_compressor_switch_pressed",
                "get_compressor_switch_pressed");

        ClassDB::bind_method(
                D_METHOD("set_converter_switch_pressed"), &TrainElectricEngine::set_converter_switch_pressed);
        ClassDB::bind_method(
                D_METHOD("get_converter_switch_pressed"), &TrainElectricEngine::get_converter_switch_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/converter"), "set_converter_switch_pressed",
                "get_converter_switch_pressed");
    }

    void TrainElectricEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        TrainEngine::_do_fetch_state_from_mover(mover, state);

        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;

        state["converter_enabled"] = mover->ConverterFlag;
        state["converted_allowed"] = mover->ConverterAllow;
        state["converter_time_to_start"] = mover->ConverterStartDelayTimer;
    }

    void TrainElectricEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainEngine::_do_update_internal_mover(mover);

        // FIXME: test data
        mover->EnginePowerSource.SourceType = TPowerSource::CurrentCollector;

        mover->CompressorSwitch(compressor_switch_pressed);
        mover->ConverterSwitch(converter_switch_pressed);
    }

    void TrainElectricEngine::set_converter_switch_pressed(const bool p_state) {
        converter_switch_pressed = p_state;
        _dirty = true;
    }
    bool TrainElectricEngine::get_converter_switch_pressed() const {
        return converter_switch_pressed;
    }
    void TrainElectricEngine::set_compressor_switch_pressed(const bool p_state) {
        compressor_switch_pressed = p_state;
        _dirty = true;
    }
    bool TrainElectricEngine::get_compressor_switch_pressed() const {
        return compressor_switch_pressed;
    }

} // namespace godot
