#include "RailVehicleSecuritySystem.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void RailVehicleSecuritySystem::_bind_methods() {
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::BOOL, aware_system_active, "aware_system");
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::BOOL, aware_system_cabsignal, "aware_system");
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::BOOL, aware_system_separate_acknowledge, "aware_system");
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::BOOL, aware_system_sifa, "aware_system");
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::FLOAT, aware_delay);
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::FLOAT, emergency_brake_delay, "emergency_brake");
        BIND_PROPERTY_W_HINT(
                RailVehicleSecuritySystem, Variant::INT, emergency_signal, PROPERTY_HINT_ENUM,
                "SIREN_LOW_TONE,SIREN_HIGH_TONE,WHISTLE");
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::BOOL, radio_stop_enabled, "radio_stop");
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::FLOAT, sound_signal_delay);
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::FLOAT, shp_magnet_distance);
        BIND_PROPERTY(RailVehicleSecuritySystem, Variant::FLOAT, ca_max_hold_time);
        ClassDB::bind_method(
                D_METHOD("security_acknowledge", "enabled"), &RailVehicleSecuritySystem::security_acknowledge);
        ClassDB::bind_method(
                D_METHOD("security_cabsignal_acknowledge"), &RailVehicleSecuritySystem::security_cabsignal_acknowledge);
        ClassDB::bind_method(
                D_METHOD("security_cabsignal_trigger"), &RailVehicleSecuritySystem::security_cabsignal_trigger);
        ClassDB::bind_method(D_METHOD("security_radiostop", "enabled"), &RailVehicleSecuritySystem::security_radiostop);
        ADD_SIGNAL(MethodInfo("blinking_changed", PropertyInfo(Variant::BOOL, "state")));
        ADD_SIGNAL(MethodInfo("beeping_changed", PropertyInfo(Variant::BOOL, "state")));

        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_SIREN_LOW_TONE);
        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_SIREN_HIGH_TONE);
        BIND_ENUM_CONSTANT(EMERGENCY_SIGNAL_WHISTLE);

        ClassDB::bind_method(D_METHOD("get_beeping"), &RailVehicleSecuritySystem::get_beeping);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "beeping", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_beeping");
        ClassDB::bind_method(D_METHOD("get_blinking"), &RailVehicleSecuritySystem::get_blinking);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "blinking", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_blinking");
        ClassDB::bind_method(D_METHOD("get_radiostop_available"), &RailVehicleSecuritySystem::get_radiostop_available);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "radiostop_available", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_radiostop_available");
        ClassDB::bind_method(D_METHOD("get_vigilance_blinking"), &RailVehicleSecuritySystem::get_vigilance_blinking);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "vigilance_blinking", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_vigilance_blinking");
        ClassDB::bind_method(D_METHOD("get_cabsignal_blinking"), &RailVehicleSecuritySystem::get_cabsignal_blinking);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "cabsignal_blinking", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabsignal_blinking");
        ClassDB::bind_method(D_METHOD("get_cabsignal_beeping"), &RailVehicleSecuritySystem::get_cabsignal_beeping);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "cabsignal_beeping", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabsignal_beeping");
        ClassDB::bind_method(D_METHOD("get_braking"), &RailVehicleSecuritySystem::get_braking);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "braking", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_braking");
        ClassDB::bind_method(D_METHOD("get_engine_blocked"), &RailVehicleSecuritySystem::get_engine_blocked);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "engine_blocked", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_engine_blocked");
        ClassDB::bind_method(
                D_METHOD("get_separate_acknowledge"), &RailVehicleSecuritySystem::get_separate_acknowledge);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "separate_acknowledge", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_separate_acknowledge");
    }

    void RailVehicleSecuritySystem::_register_commands() {
        register_command("security_acknowledge", Callable(this, "security_acknowledge"));
        register_command("security_cabsignal_acknowledge", Callable(this, "security_cabsignal_acknowledge"));
        register_command("security_cabsignal_trigger", Callable(this, "security_cabsignal_trigger"));
        register_command("security_radiostop", Callable(this, "security_radiostop"));
    }

    void RailVehicleSecuritySystem::_unregister_commands() {
        unregister_command("security_acknowledge");
        unregister_command("security_cabsignal_acknowledge");
        unregister_command("security_cabsignal_trigger");
        unregister_command("security_radiostop");
    }
} // namespace godot
