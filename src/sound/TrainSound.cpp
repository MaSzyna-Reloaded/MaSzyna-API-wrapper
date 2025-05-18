#include "TrainSound.hpp"
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <utility>

namespace godot {
    void TrainSound::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_max_volume_db", "max_volume_db"), &TrainSound::set_max_volume_db);
        ClassDB::bind_method(D_METHOD("get_max_volume_db"), &TrainSound::get_max_volume_db);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_volume_db"), "set_max_volume_db", "get_max_volume_db");
        ClassDB::bind_method(D_METHOD("set_min_volume_db", "min_volume_db"), &TrainSound::set_min_volume_db);
        ClassDB::bind_method(D_METHOD("get_min_volume_db"), &TrainSound::get_min_volume_db);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_volume_db"), "set_min_volume_db", "get_min_volume_db");
        ClassDB::bind_method(D_METHOD("set_sound_root_path", "path"), &TrainSound::set_sound_root_path);
        ClassDB::bind_method(D_METHOD("get_sound_root_path"), &TrainSound::get_sound_root_path);
        ADD_PROPERTY(
                PropertyInfo(Variant::STRING, "sound_root_path", PROPERTY_HINT_DIR), "set_sound_root_path",
                "get_sound_root_path");
        ClassDB::bind_method(
                D_METHOD(
                        "update_audio_streams_and_volumes", "param_values", "layered_sound_data", "player_1",
                        "player_2"),
                &TrainSound::update_audio_streams_and_volumes);

        ClassDB::bind_method(D_METHOD("set_battery_sounds", "sounds"), &TrainSound::set_battery_sounds);
        ClassDB::bind_method(D_METHOD("get_battery_sounds"), &TrainSound::get_battery_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "battery/sounds", PROPERTY_HINT_TYPE_STRING,
                        String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) +
                                ":LayeredSoundResource",
                        PROPERTY_USAGE_DEFAULT, "Ref<LayeredSoundResource>"),
                "set_battery_sounds", "get_battery_sounds");

        ClassDB::bind_method(D_METHOD("set_brake_sounds", "sounds"), &TrainSound::set_brake_sounds);
        ClassDB::bind_method(D_METHOD("get_brake_sounds"), &TrainSound::get_brake_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "brake/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_brake_sounds", "get_brake_sounds");

        ClassDB::bind_method(D_METHOD("set_brake_acc_sounds", "sounds"), &TrainSound::set_brake_acc_sounds);
        ClassDB::bind_method(D_METHOD("get_brake_acc_sounds"), &TrainSound::get_brake_acc_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "brake/acc/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_brake_acc_sounds", "get_brake_acc_sounds");

        ClassDB::bind_method(
                D_METHOD("set_brake_cylinder_inc_sounds", "sounds"), &TrainSound::set_brake_cylinder_inc_sounds);
        ClassDB::bind_method(D_METHOD("get_brake_cylinder_inc_sounds"), &TrainSound::get_brake_cylinder_inc_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "brake/cylinder/inc/sounds", PROPERTY_HINT_RESOURCE_TYPE,
                        "LayeredSoundResource"),
                "set_brake_cylinder_inc_sounds", "get_brake_cylinder_inc_sounds");

        ClassDB::bind_method(
                D_METHOD("set_brake_cylinder_dec_sounds", "sounds"), &TrainSound::set_brake_cylinder_dec_sounds);
        ClassDB::bind_method(D_METHOD("get_brake_cylinder_dec_sounds"), &TrainSound::get_brake_cylinder_dec_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "brake/cylinder/dec/sounds", PROPERTY_HINT_RESOURCE_TYPE,
                        "LayeredSoundResource"),
                "set_brake_cylinder_dec_sounds", "get_brake_cylinder_dec_sounds");

        ClassDB::bind_method(
                D_METHOD("set_brake_hose_attach_sounds", "sounds"), &TrainSound::set_brake_hose_attach_sounds);
        ClassDB::bind_method(D_METHOD("get_brake_hose_attach_sounds"), &TrainSound::get_brake_hose_attach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "brake/hose/attach/sounds", PROPERTY_HINT_RESOURCE_TYPE,
                        "LayeredSoundResource"),
                "set_brake_hose_attach_sounds", "get_brake_hose_attach_sounds");

        ClassDB::bind_method(
                D_METHOD("set_brake_hose_detach_sounds", "sounds"), &TrainSound::set_brake_hose_detach_sounds);
        ClassDB::bind_method(D_METHOD("get_brake_hose_detach_sounds"), &TrainSound::get_brake_hose_detach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "brake/hose/detach/sounds", PROPERTY_HINT_RESOURCE_TYPE,
                        "LayeredSoundResource"),
                "set_brake_hose_detach_sounds", "get_brake_hose_detach_sounds");

        ClassDB::bind_method(D_METHOD("set_compressor_sounds", "sounds"), &TrainSound::set_compressor_sounds);
        ClassDB::bind_method(D_METHOD("get_compressor_sounds"), &TrainSound::get_compressor_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "compressor/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_compressor_sounds", "get_compressor_sounds");

        ClassDB::bind_method(D_METHOD("set_control_attach_sounds", "sounds"), &TrainSound::set_control_attach_sounds);
        ClassDB::bind_method(D_METHOD("get_control_attach_sounds"), &TrainSound::get_control_attach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "control/attach/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_control_attach_sounds", "get_control_attach_sounds");

        ClassDB::bind_method(D_METHOD("set_control_detach_sounds", "sounds"), &TrainSound::set_control_detach_sounds);
        ClassDB::bind_method(D_METHOD("get_control_detach_sounds"), &TrainSound::get_control_detach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "control/detach/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_control_detach_sounds", "get_control_detach_sounds");

        ClassDB::bind_method(D_METHOD("set_converter_sounds", "sounds"), &TrainSound::set_converter_sounds);
        ClassDB::bind_method(D_METHOD("get_converter_sounds"), &TrainSound::get_converter_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "converter/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_converter_sounds", "get_converter_sounds");

        ClassDB::bind_method(D_METHOD("set_curve_sounds", "sounds"), &TrainSound::set_curve_sounds);
        ClassDB::bind_method(D_METHOD("get_curve_sounds"), &TrainSound::get_curve_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "curve/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_curve_sounds", "get_curve_sounds");

        ClassDB::bind_method(
                D_METHOD("set_departure_signal_sounds", "sounds"), &TrainSound::set_departure_signal_sounds);
        ClassDB::bind_method(D_METHOD("get_departure_signal_sounds"), &TrainSound::get_departure_signal_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "departure_signal/sounds", PROPERTY_HINT_RESOURCE_TYPE,
                        "LayeredSoundResource"),
                "set_departure_signal_sounds", "get_departure_signal_sounds");

        ClassDB::bind_method(D_METHOD("set_derail_sounds", "sounds"), &TrainSound::set_derail_sounds);
        ClassDB::bind_method(D_METHOD("get_derail_sounds"), &TrainSound::get_derail_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "derail/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_derail_sounds", "get_derail_sounds");

        ClassDB::bind_method(D_METHOD("set_diesel_inc_sounds", "sounds"), &TrainSound::set_diesel_inc_sounds);
        ClassDB::bind_method(D_METHOD("get_diesel_inc_sounds"), &TrainSound::get_diesel_inc_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "diesel_inc/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_diesel_inc_sounds", "get_diesel_inc_sounds");

        ClassDB::bind_method(D_METHOD("set_door_close_sounds", "sounds"), &TrainSound::set_door_close_sounds);
        ClassDB::bind_method(D_METHOD("get_door_close_sounds"), &TrainSound::get_door_close_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "door/close/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_close_sounds", "get_door_close_sounds");

        ClassDB::bind_method(D_METHOD("set_door_open_sounds", "sounds"), &TrainSound::set_door_open_sounds);
        ClassDB::bind_method(D_METHOD("get_door_open_sounds"), &TrainSound::get_door_open_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "door/open/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_open_sounds", "get_door_open_sounds");
        ClassDB::bind_method(D_METHOD("set_door_lock_sounds", "sounds"), &TrainSound::set_door_lock_sounds);
        ClassDB::bind_method(D_METHOD("get_door_lock_sounds"), &TrainSound::get_door_lock_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "door/lock/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_lock_sounds", "get_door_lock_sounds");

        ClassDB::bind_method(D_METHOD("set_door_step_open_sounds", "sounds"), &TrainSound::set_door_step_open_sounds);
        ClassDB::bind_method(D_METHOD("get_door_step_open_sounds"), &TrainSound::get_door_step_open_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "door/step/open/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_step_open_sounds", "get_door_step_open_sounds");

        ClassDB::bind_method(D_METHOD("set_door_step_close_sounds", "sounds"), &TrainSound::set_door_step_close_sounds);
        ClassDB::bind_method(D_METHOD("get_door_step_close_sounds"), &TrainSound::get_door_step_close_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "door/step/close/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_step_close_sounds", "get_door_step_close_sounds");

        ClassDB::bind_method(D_METHOD("set_door_run_lock_sounds", "sounds"), &TrainSound::set_door_run_lock_sounds);
        ClassDB::bind_method(D_METHOD("get_door_run_lock_sounds"), &TrainSound::get_door_run_lock_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "door/run_lock/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_run_lock_sounds", "get_door_run_lock_sounds");

        ClassDB::bind_method(D_METHOD("set_door_permit_sounds", "sounds"), &TrainSound::set_door_permit_sounds);
        ClassDB::bind_method(D_METHOD("get_door_permit_sounds"), &TrainSound::get_door_permit_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "door/permit/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_door_permit_sounds", "get_door_permit_sounds");

        ClassDB::bind_method(D_METHOD("set_emergency_brake_sounds", "sounds"), &TrainSound::set_emergency_brake_sounds);
        ClassDB::bind_method(D_METHOD("get_emergency_brake_sounds"), &TrainSound::get_emergency_brake_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "emergency_brake/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_emergency_brake_sounds", "get_emergency_brake_sounds");

        ClassDB::bind_method(D_METHOD("set_engine_sounds", "sounds"), &TrainSound::set_engine_sounds);
        ClassDB::bind_method(D_METHOD("get_engine_sounds"), &TrainSound::get_engine_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "engine/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_engine_sounds", "get_engine_sounds");

        ClassDB::bind_method(D_METHOD("set_ep_brake_inc_sounds", "sounds"), &TrainSound::set_ep_brake_inc_sounds);
        ClassDB::bind_method(D_METHOD("get_ep_brake_inc_sounds"), &TrainSound::get_ep_brake_inc_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "ep_brake/inc/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_ep_brake_inc_sounds", "get_ep_brake_inc_sounds");

        ClassDB::bind_method(D_METHOD("set_ep_brake_dec_sounds", "sounds"), &TrainSound::set_ep_brake_dec_sounds);
        ClassDB::bind_method(D_METHOD("get_ep_brake_dec_sounds"), &TrainSound::get_ep_brake_dec_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "ep_brake/dec/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_ep_brake_dec_sounds", "get_ep_brake_dec_sounds");

        ClassDB::bind_method(D_METHOD("set_ep_ctrl_value_sounds", "sounds"), &TrainSound::set_ep_ctrl_value_sounds);
        ClassDB::bind_method(D_METHOD("get_ep_ctrl_value_sounds"), &TrainSound::get_ep_ctrl_value_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "ep_ctrl_value/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_ep_ctrl_value_sounds", "get_ep_ctrl_value_sounds");

        ClassDB::bind_method(D_METHOD("set_fuel_pump_sounds", "sounds"), &TrainSound::set_fuel_pump_sounds);
        ClassDB::bind_method(D_METHOD("get_fuel_pump_sounds"), &TrainSound::get_fuel_pump_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "fuel_pump/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_fuel_pump_sounds", "get_fuel_pump_sounds");

        ClassDB::bind_method(D_METHOD("set_gangway_attach_sounds", "sounds"), &TrainSound::set_gangway_attach_sounds);
        ClassDB::bind_method(D_METHOD("get_gangway_attach_sounds"), &TrainSound::get_gangway_attach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "gangway/attach/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_gangway_attach_sounds", "get_gangway_attach_sounds");

        ClassDB::bind_method(D_METHOD("set_gangway_detach_sounds", "sounds"), &TrainSound::set_gangway_detach_sounds);
        ClassDB::bind_method(D_METHOD("get_gangway_detach_sounds"), &TrainSound::get_gangway_detach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "gangway/detach/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_gangway_detach_sounds", "get_gangway_detach_sounds");

        ClassDB::bind_method(D_METHOD("set_heater_sounds", "sounds"), &TrainSound::set_heater_sounds);
        ClassDB::bind_method(D_METHOD("get_heater_sounds"), &TrainSound::get_heater_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "heater/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_heater_sounds", "get_heater_sounds");

        ClassDB::bind_method(D_METHOD("set_heating_attach_sounds", "sounds"), &TrainSound::set_heating_attach_sounds);
        ClassDB::bind_method(D_METHOD("get_heating_attach_sounds"), &TrainSound::get_heating_attach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "heating/attach/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_heating_attach_sounds", "get_heating_attach_sounds");

        ClassDB::bind_method(D_METHOD("set_heating_detach_sounds", "sounds"), &TrainSound::set_heating_detach_sounds);
        ClassDB::bind_method(D_METHOD("get_heating_detach_sounds"), &TrainSound::get_heating_detach_sounds);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "heating/detach/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_heating_detach_sounds", "get_heating_detach_sounds");

        ClassDB::bind_method(D_METHOD("set_horn_high_sounds", "sounds"), &TrainSound::set_horn_high_sounds);
        ClassDB::bind_method(D_METHOD("get_horn_high_sounds"), &TrainSound::get_horn_high_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "horn/high/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_horn_high_sounds", "get_horn_high_sounds");

        ClassDB::bind_method(D_METHOD("set_horn_low_sounds", "sounds"), &TrainSound::set_horn_low_sounds);
        ClassDB::bind_method(D_METHOD("get_horn_low_sounds"), &TrainSound::get_horn_low_sounds);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "horn/low/sounds", PROPERTY_HINT_RESOURCE_TYPE, "LayeredSoundResource"),
                "set_horn_low_sounds", "get_horn_low_sounds");
        ClassDB::bind_method(
                D_METHOD("get_audio_stream_for_file", "file_path", "loop"), &TrainSound::get_audio_stream_for_file);
    }

    void TrainSound::set_battery_sounds(const Ref<LayeredSoundResource> &sounds) {
        battery.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_battery_sounds() {
        return battery.sounds;
    }
    void TrainSound::set_brake_sounds(const Ref<LayeredSoundResource> &sounds) {
        brake.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_brake_sounds() {
        return brake.sounds;
    }
    void TrainSound::set_brake_acc_sounds(const Ref<LayeredSoundResource> &sounds) {
        brake_acc.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_brake_acc_sounds() {
        return brake_acc.sounds;
    }
    void TrainSound::set_brake_cylinder_inc_sounds(const Ref<LayeredSoundResource> &sounds) {
        brake_cylinder_inc.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_brake_cylinder_inc_sounds() {
        return brake_cylinder_inc.sounds;
    }
    void TrainSound::set_brake_cylinder_dec_sounds(const Ref<LayeredSoundResource> &sounds) {
        brake_cylinder_dec.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_brake_cylinder_dec_sounds() {
        return brake_cylinder_dec.sounds;
    }
    void TrainSound::set_brake_hose_attach_sounds(const Ref<LayeredSoundResource> &sounds) {
        brake_hose_attach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_brake_hose_attach_sounds() {
        return brake_hose_attach.sounds;
    }
    void TrainSound::set_brake_hose_detach_sounds(const Ref<LayeredSoundResource> &sounds) {
        brake_hose_detach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_brake_hose_detach_sounds() {
        return brake_hose_detach.sounds;
    }
    void TrainSound::set_compressor_sounds(const Ref<LayeredSoundResource> &sounds) {
        compressor.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_compressor_sounds() {
        return compressor.sounds;
    }
    void TrainSound::set_control_attach_sounds(const Ref<LayeredSoundResource> &sounds) {
        control_attach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_control_attach_sounds() {
        return control_attach.sounds;
    }
    void TrainSound::set_control_detach_sounds(const Ref<LayeredSoundResource> &sounds) {
        control_detach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_control_detach_sounds() {
        return control_detach.sounds;
    }
    void TrainSound::set_converter_sounds(const Ref<LayeredSoundResource> &sounds) {
        converter.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_converter_sounds() {
        return converter.sounds;
    }
    void TrainSound::set_curve_sounds(const Ref<LayeredSoundResource> &sounds) {
        curve.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_curve_sounds() {
        return curve.sounds;
    }
    void TrainSound::set_departure_signal_sounds(const Ref<LayeredSoundResource> &sounds) {
        departure_signal.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_departure_signal_sounds() {
        return departure_signal.sounds;
    }
    void TrainSound::set_derail_sounds(const Ref<LayeredSoundResource> &sounds) {
        derail.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_derail_sounds() {
        return derail.sounds;
    }
    void TrainSound::set_diesel_inc_sounds(const Ref<LayeredSoundResource> &sounds) {
        diesel_inc.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_diesel_inc_sounds() {
        return diesel_inc.sounds;
    }
    void TrainSound::set_door_close_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_close.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_close_sounds() {
        return door_close.sounds;
    }
    void TrainSound::set_door_open_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_open.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_open_sounds() {
        return door_open.sounds;
    }
    void TrainSound::set_door_lock_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_lock.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_lock_sounds() {
        return door_lock.sounds;
    }
    void TrainSound::set_door_step_open_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_step_open.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_step_open_sounds() {
        return door_step_open.sounds;
    }
    void TrainSound::set_door_step_close_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_step_close.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_step_close_sounds() {
        return door_step_close.sounds;
    }
    void TrainSound::set_door_run_lock_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_run_lock.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_run_lock_sounds() {
        return door_run_lock.sounds;
    }
    void TrainSound::set_door_permit_sounds(const Ref<LayeredSoundResource> &sounds) {
        door_permit.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_door_permit_sounds() {
        return door_permit.sounds;
    }
    void TrainSound::set_emergency_brake_sounds(const Ref<LayeredSoundResource> &sounds) {
        emergency_brake.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_emergency_brake_sounds() {
        return emergency_brake.sounds;
    }
    void TrainSound::set_engine_sounds(const Ref<LayeredSoundResource> &sounds) {
        engine.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_engine_sounds() {
        return engine.sounds;
    }
    void TrainSound::set_ep_brake_inc_sounds(const Ref<LayeredSoundResource> &sounds) {
        ep_brake_inc.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_ep_brake_inc_sounds() {
        return ep_brake_inc.sounds;
    }
    void TrainSound::set_ep_brake_dec_sounds(const Ref<LayeredSoundResource> &sounds) {
        ep_brake_dec.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_ep_brake_dec_sounds() {
        return ep_brake_dec.sounds;
    }
    void TrainSound::set_ep_ctrl_value_sounds(const Ref<LayeredSoundResource> &sounds) {
        ep_ctrl_value.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_ep_ctrl_value_sounds() {
        return ep_ctrl_value.sounds;
    }
    void TrainSound::set_fuel_pump_sounds(const Ref<LayeredSoundResource> &sounds) {
        fuel_pump.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_fuel_pump_sounds() {
        return fuel_pump.sounds;
    }
    void TrainSound::set_gangway_attach_sounds(const Ref<LayeredSoundResource> &sounds) {
        gangway_attach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_gangway_attach_sounds() {
        return gangway_attach.sounds;
    }
    void TrainSound::set_gangway_detach_sounds(const Ref<LayeredSoundResource> &sounds) {
        gangway_detach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_gangway_detach_sounds() {
        return gangway_detach.sounds;
    }
    void TrainSound::set_heater_sounds(const Ref<LayeredSoundResource> &sounds) {
        heater.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_heater_sounds() {
        return heater.sounds;
    }
    void TrainSound::set_heating_attach_sounds(const Ref<LayeredSoundResource> &sounds) {
        heating_attach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_heating_attach_sounds() {
        return heating_attach.sounds;
    }
    void TrainSound::set_heating_detach_sounds(const Ref<LayeredSoundResource> &sounds) {
        heating_detach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_heating_detach_sounds() {
        return heating_detach.sounds;
    }
    void TrainSound::set_horn_high_sounds(const Ref<LayeredSoundResource> &sounds) {
        horn_high.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_horn_high_sounds() {
        return horn_high.sounds;
    }
    void TrainSound::set_horn_low_sounds(const Ref<LayeredSoundResource> &sounds) {
        horn_low.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_horn_low_sounds() {
        return horn_low.sounds;
    }
    void TrainSound::set_whistle_sounds(const Ref<LayeredSoundResource> &sounds) {
        whistle.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_whistle_sounds() {
        return whistle.sounds;
    }
    void TrainSound::set_inverter_sounds(const Ref<LayeredSoundResource> &sounds) {
        inverter.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_inverter_sounds() {
        return inverter.sounds;
    }
    void TrainSound::set_loading_sounds(const Ref<LayeredSoundResource> &sounds) {
        loading.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_loading_sounds() {
        return loading.sounds;
    }
    void TrainSound::set_main_hose_attach_sounds(const Ref<LayeredSoundResource> &sounds) {
        main_hose_attach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_main_hose_attach_sounds() {
        return main_hose_attach.sounds;
    }
    void TrainSound::set_main_hose_detach_sounds(const Ref<LayeredSoundResource> &sounds) {
        main_hose_detach.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_main_hose_detach_sounds() {
        return main_hose_detach.sounds;
    }
    void TrainSound::set_motor_blower_sounds(const Ref<LayeredSoundResource> &sounds) {
        motor_blower.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_motor_blower_sounds() {
        return motor_blower.sounds;
    }
    void TrainSound::set_pantograph_up_sounds(const Ref<LayeredSoundResource> &sounds) {
        pantograph_up.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_pantograph_up_sounds() {
        return pantograph_up.sounds;
    }
    void TrainSound::set_pantograph_down_sounds(const Ref<LayeredSoundResource> &sounds) {
        pantograph_down.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_pantograph_down_sounds() {
        return pantograph_down.sounds;
    }
    void TrainSound::set_sand_sounds(const Ref<LayeredSoundResource> &sounds) {
        sand.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_sand_sounds() {
        return sand.sounds;
    }
    void TrainSound::set_small_compressor_sounds(const Ref<LayeredSoundResource> &sounds) {
        small_compressor.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_small_compressor_sounds() {
        return small_compressor.sounds;
    }
    void TrainSound::set_start_jolt_sounds(const Ref<LayeredSoundResource> &sounds) {
        start_jolt.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_start_jolt_sounds() {
        return start_jolt.sounds;
    }
    void TrainSound::set_traction_motor_sounds(const Ref<LayeredSoundResource> &sounds) {
        traction_motor.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_traction_motor_sounds() {
        return traction_motor.sounds;
    }
    void TrainSound::set_traction_motor_ac_sounds(const Ref<LayeredSoundResource> &sounds) {
        traction_motor_ac.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_traction_motor_ac_sounds() {
        return traction_motor_ac.sounds;
    }
    void TrainSound::set_transmission_sounds(const Ref<LayeredSoundResource> &sounds) {
        transmission.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_transmission_sounds() {
        return transmission.sounds;
    }
    void TrainSound::set_turbo_sounds(const Ref<LayeredSoundResource> &sounds) {
        turbo.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_turbo_sounds() {
        return turbo.sounds;
    }
    void TrainSound::set_ventilator_sounds(const Ref<LayeredSoundResource> &sounds) {
        ventilator.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_ventilator_sounds() {
        return ventilator.sounds;
    }
    void TrainSound::set_un_brake_sounds(const Ref<LayeredSoundResource> &sounds) {
        un_brake.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_un_brake_sounds() {
        return un_brake.sounds;
    }
    void TrainSound::set_un_loading_sounds(const Ref<LayeredSoundResource> &sounds) {
        un_loading.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_un_loading_sounds() {
        return un_loading.sounds;
    }
    void TrainSound::set_wheel_flat_sounds(const Ref<LayeredSoundResource> &sounds) {
        wheel_flat.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_wheel_flat_sounds() {
        return wheel_flat.sounds;
    }
    void TrainSound::set_wheel_clatter_sounds(const Ref<LayeredSoundResource> &sounds) {
        wheel_clatter.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_wheel_clatter_sounds() {
        return wheel_clatter.sounds;
    }
    void TrainSound::set_water_pump_sounds(const Ref<LayeredSoundResource> &sounds) {
        water_pump.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_water_pump_sounds() {
        return water_pump.sounds;
    }
    void TrainSound::set_water_heater_sounds(const Ref<LayeredSoundResource> &sounds) {
        water_heater.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_water_heater_sounds() {
        return water_heater.sounds;
    }
    void TrainSound::set_compressor_idle_sounds(const Ref<LayeredSoundResource> &sounds) {
        compressor_idle.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_compressor_idle_sounds() {
        return compressor_idle.sounds;
    }
    void TrainSound::set_braking_resistor_ventilator_sounds(const Ref<LayeredSoundResource> &sounds) {
        braking_resistor_ventilator.sounds = sounds;
    }
    Ref<LayeredSoundResource> TrainSound::get_braking_resistor_ventilator_sounds() {
        return braking_resistor_ventilator.sounds;
    }

    Ref<AudioStream> TrainSound::get_audio_stream_for_file(const String &p_file_path, bool p_loop_stream) {
        if (!FileAccess::file_exists(p_file_path)) {
            UtilityFunctions::push_error("[TrainSound] Audio file not found at path: " + p_file_path);
            return nullptr;
        }

        if (stream_cache.find(p_file_path) != stream_cache.end()) {
            UtilityFunctions::print_verbose("[TrainSound] Returning already preloaded stream for: " + p_file_path);
            return stream_cache[p_file_path];
        }

        UtilityFunctions::print_verbose("[TrainSound] Generating AudioStream stream for: " + p_file_path);
        Ref<AudioStream> _audio_steam = nullptr;
        if (const String extension = p_file_path.get_extension().to_lower(); extension == "wav") {
            Ref<AudioStreamWAV> _stream = AudioStreamWAV::load_from_file(p_file_path);
            if (p_loop_stream) {
                _stream->set_loop_mode(AudioStreamWAV::LOOP_FORWARD);
            } else {
                _stream->set_loop_mode(AudioStreamWAV::LOOP_DISABLED);
            }
            _audio_steam = _stream;
        } else if (extension == "ogg") {
            Ref<AudioStreamOggVorbis> _stream = AudioStreamOggVorbis::load_from_file(p_file_path);
            _stream->set_loop(p_loop_stream);
            _audio_steam = _stream;
        } else if (extension == "mp3") {
            Ref<AudioStreamMP3> _stream = AudioStreamMP3::load_from_file(p_file_path);
            _stream->set_loop(p_loop_stream);
            _audio_steam = _stream;
        } else {
            UtilityFunctions::push_warning("[TrainSound] Unsupported file extension for: " + p_file_path);
            return nullptr;
        }

        stream_cache[p_file_path] = _audio_steam;
        return _audio_steam;
    }

    void TrainSound::update_audio_streams_and_volumes(
            double param_value, const Ref<LayeredSoundResource> &layered_sound_data, AudioStreamPlayer3D *player_1,
            AudioStreamPlayer3D *player_2) {
        Array sorted_parameter_keys = layered_sound_data->sound_table.keys();
        Dictionary audio_map = layered_sound_data->sound_table;
        double crossfade_threshold_percent = layered_sound_data->cross_fade;
        if (SOUND_ROOT_PATH == "") {
            UtilityFunctions::push_error("[TrainSound] Sound root path not set!");
            return;
        }

        if (!SOUND_ROOT_PATH.ends_with("/")) {
            UtilityFunctions::push_error("[TrainSound] Sound root path must end with a slash!");
            return;
        }

        if (player_1 == nullptr || player_2 == nullptr) {
            UtilityFunctions::push_error("[TrainSound] One of required players is not assigned.");
            return;
        }

        int idx_min = -1;
        for (int i = static_cast<int>(sorted_parameter_keys.size()) - 1; i >= 0; --i) {
            if (param_value >= static_cast<double>(sorted_parameter_keys[i])) {
                idx_min = i;
                break;
            }
        }

        if (idx_min == -1) {
            idx_min = 0;
        }

        int param_min = static_cast<int>(sorted_parameter_keys[idx_min]);
        if (!audio_map.has(param_min)) {
            UtilityFunctions::push_error("TrainSound: Key ", Variant(param_min), " not found in audio_map.");
            return;
        }

        String file_path_min = SOUND_ROOT_PATH + static_cast<String>(audio_map[param_min]);
        Ref<AudioStream> stream_min = get_audio_stream_for_file(file_path_min, true);
        int param_max = 0;
        Ref<AudioStream> stream_max;
        int idx_max = idx_min + 1;
        if (idx_max < sorted_parameter_keys.size()) {
            param_max = static_cast<int>(sorted_parameter_keys[idx_max]);
            if (!audio_map.has(param_max)) {
                UtilityFunctions::push_error("TrainSound: Key ", Variant(param_max), " not found in audio_map.");
                return;
            }
            String file_path_max = SOUND_ROOT_PATH + static_cast<String>(audio_map[param_max]);
            stream_max = get_audio_stream_for_file(file_path_max, true);
        } else {
            param_max = param_min + 1;
        }

        AudioStreamPlayer3D *p_min = nullptr;
        AudioStreamPlayer3D *p_max = nullptr;
        if (idx_min % 2 == 0) {
            p_min = player_1;
            p_max = player_2;
        } else {
            p_min = player_2;
            p_max = player_1;
        }

        if (p_min->get_stream() != stream_min) {
            p_min->set_stream(stream_min);
            p_min->set_max_distance(static_cast<float>(layered_sound_data->range));
            UtilityFunctions::print_verbose(
                    "Assigned stream_min (" + stream_min->get_path().get_file() + ") to " + p_min->get_name());
            if (!p_min->is_playing()) {
                p_min->play();
            }
        }

        if (stream_max != nullptr) {
            if (p_max->get_stream() != stream_max) {
                p_max->set_stream(stream_max);
                p_max->set_max_distance(static_cast<float>(layered_sound_data->range));
                UtilityFunctions::print_verbose(
                        "Assigned stream_max (" + stream_max->get_path().get_file() + ") to " + p_max->get_name());
                if (!p_max->is_playing()) {
                    p_max->play();
                }
            }
        } else {
            if (p_max->get_stream() != nullptr) {
                p_max->stop();
                p_max->set_stream(nullptr);
                UtilityFunctions::print_verbose("Cleared stream from " + p_max->get_name() + " (highest RPM range)");
            }
        }

        double progress = 0.0;
        if (stream_max != nullptr && param_max > param_min) {
            if (crossfade_threshold_percent > 100 || crossfade_threshold_percent < 0) {
                UtilityFunctions::push_error(
                        "[TrainSound] Crossfade threshold must be within a range between 0 and 100");
                return;
            }

            double crossfade_start = param_min + ((param_max - param_min) * (crossfade_threshold_percent / 100.0));
            if (param_value >= crossfade_start) {
                // Thanks a lot for help, LeD
                progress = Math::clamp(Math::inverse_lerp(param_min, param_max, param_value), 0.0, 1.0);
                double vol_min_linear = 1.0 - progress;
                double vol_max_linear = progress;
                p_min->set_volume_db(static_cast<float>(UtilityFunctions::linear_to_db(vol_min_linear)));
                p_max->set_volume_db(static_cast<float>(UtilityFunctions::linear_to_db(vol_max_linear)));
            }
        } else {
            p_min->set_volume_db(MAX_VOLUME_DB);
            p_max->set_volume_db(MIN_VOLUME_DB);
        }
    }
    String TrainSound::get_sound_root_path() {
        return SOUND_ROOT_PATH;
    }
    void TrainSound::set_max_volume_db(float max_volume_db) {
        MAX_VOLUME_DB = max_volume_db;
    }
    float TrainSound::get_max_volume_db() {
        return MAX_VOLUME_DB;
    }
    void TrainSound::set_min_volume_db(float min_volume_db) {
        MIN_VOLUME_DB = min_volume_db;
    }
    float TrainSound::get_min_volume_db() {
        return MIN_VOLUME_DB;
    }
    void TrainSound::set_sound_root_path(String sound_root_path) {
        SOUND_ROOT_PATH = std::move(sound_root_path);
    }
} // namespace godot
