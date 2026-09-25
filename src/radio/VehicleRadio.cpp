#include "VehicleRadio.hpp"

namespace godot {
    const char *VehicleRadio::radio_toggled_signal = "radio_toggled";
    const char *VehicleRadio::channel_changed_signal = "channel_changed";

    void VehicleRadio::_bind_methods() {
        BIND_PROPERTY(VehicleRadio, Variant::INT, channel_min, "channel");
        BIND_PROPERTY(VehicleRadio, Variant::INT, channel_max, "channel");

        ClassDB::bind_method(D_METHOD("radio", "enabled"), &VehicleRadio::radio);
        ClassDB::bind_method(D_METHOD("channel_set", "channel"), &VehicleRadio::channel_set);
        ClassDB::bind_method(D_METHOD("channel_increase", "step"), &VehicleRadio::channel_increase, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("channel_decrease", "step"), &VehicleRadio::channel_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("volume_increase", "step"), &VehicleRadio::volume_increase, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("volume_decrease", "step"), &VehicleRadio::volume_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("radio_stop", "pressed"), &VehicleRadio::radio_stop);

        ClassDB::bind_method(D_METHOD("get_enabled"), &VehicleRadio::get_enabled);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "enabled", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_enabled");
        ClassDB::bind_method(D_METHOD("get_powered"), &VehicleRadio::get_powered);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "powered", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_powered");
        ClassDB::bind_method(D_METHOD("get_channel"), &VehicleRadio::get_channel);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "channel", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_channel");
        ClassDB::bind_method(D_METHOD("get_volume"), &VehicleRadio::get_volume);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "volume", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_volume");

        ADD_SIGNAL(MethodInfo(radio_toggled_signal, PropertyInfo(Variant::BOOL, "enabled")));
        ADD_SIGNAL(MethodInfo(channel_changed_signal, PropertyInfo(Variant::INT, "channel")));
    }

    int VehicleRadio::get_channel() const {
        return channel;
    }

    double VehicleRadio::get_volume() const {
        return volume;
    }

    void VehicleRadio::channel_set(const int p_channel) {
        const int new_channel = Math::clamp(p_channel, get_channel_min(), get_channel_max());
        if (new_channel == channel) {
            return;
        }
        channel = new_channel;
        emit_signal(channel_changed_signal, channel);
    }

    void VehicleRadio::channel_increase(const int p_step) {
        channel_set(channel + (p_step > 0 ? p_step : 1));
    }

    void VehicleRadio::channel_decrease(const int p_step) {
        channel_set(channel - (p_step != 0 ? p_step : 1));
    }

    // Original engine: TTrain::OnCommand_radiovolumeincrease/decrease -> radiovolumeset
    // (Train.cpp:8246-8289), clamped to 0..1
    void VehicleRadio::volume_increase(const int p_step) {
        volume = Math::clamp(volume + VOLUME_STEP * (p_step > 0 ? p_step : 1), 0.0, 1.0);
    }

    void VehicleRadio::volume_decrease(const int p_step) {
        volume = Math::clamp(volume - VOLUME_STEP * (p_step > 0 ? p_step : 1), 0.0, 1.0);
    }

    void VehicleRadio::_fill_state_dictionary(Dictionary &p_state) const {
        p_state["radio_enabled"] = get_enabled();
        p_state["radio_powered"] = get_powered();
        p_state["radio_channel"] = get_channel();
        p_state["radio_volume"] = get_volume();
    }

    void VehicleRadio::_register_commands() {
        VehicleComponent::_register_commands();
        register_command("radio", Callable(this, "radio"));
        register_command("radio_channel_set", Callable(this, "channel_set"));
        register_command("radio_channel_increase", Callable(this, "channel_increase"));
        register_command("radio_channel_decrease", Callable(this, "channel_decrease"));
        register_command("radio_volume_increase", Callable(this, "volume_increase"));
        register_command("radio_volume_decrease", Callable(this, "volume_decrease"));
        register_command("radio_stop", Callable(this, "radio_stop"));
    }

    void VehicleRadio::_unregister_commands() {
        VehicleComponent::_unregister_commands();
        unregister_command("radio", Callable(this, "radio"));
        unregister_command("radio_channel_set", Callable(this, "channel_set"));
        unregister_command("radio_channel_increase", Callable(this, "channel_increase"));
        unregister_command("radio_channel_decrease", Callable(this, "channel_decrease"));
        unregister_command("radio_volume_increase", Callable(this, "volume_increase"));
        unregister_command("radio_volume_decrease", Callable(this, "volume_decrease"));
        unregister_command("radio_stop", Callable(this, "radio_stop"));
    }
} // namespace godot
