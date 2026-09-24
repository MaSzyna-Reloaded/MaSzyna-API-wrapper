#include "../core/VehicleController.hpp"
#include "../core/VehicleComponent.hpp"
#include "../core/TrainSystem.hpp"
#include "../engines/VehicleEngine.hpp"
#include "../lighting/VehicleLighting.hpp"
#include "../physics/RailVehicleServer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *VehicleController::mover_config_changed_signal = "mover_config_changed";
    const char *VehicleController::mover_initialized_signal = "mover_initialized";
    const char *VehicleController::power_changed_signal = "power_changed";
    const char *VehicleController::command_received = "command_received";
    const char *VehicleController::radio_toggled = "radio_toggled";
    const char *VehicleController::radio_channel_changed = "radio_channel_changed";
    const char *VehicleController::roof_light_changed = "roof_light_changed";
    const char *VehicleController::cabin_occupied_changed = "cabin_occupied_changed";
    const char *VehicleController::config_changed = "config_changed";
    const char *VehicleController::position_changed_signal = "position_changed";
    const char *VehicleController::consist_changed_signal = "consist_changed";
    const char *VehicleController::coupler_attached_signal = "coupler_attached";
    const char *VehicleController::coupler_detached_signal = "coupler_detached";

    void VehicleController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_state"), &VehicleController::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &VehicleController::get_config);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "state", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_state");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "config", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_config");

        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &VehicleController::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));

        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &VehicleController::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));


        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &VehicleController::register_command);
        ClassDB::bind_method(
                D_METHOD("unregister_command", "command", "callable"), &VehicleController::unregister_command);
        ClassDB::bind_method(D_METHOD("battery", "enabled"), &VehicleController::battery);
        ClassDB::bind_method(D_METHOD("cab_activation", "enabled"), &VehicleController::cab_activation);
        ClassDB::bind_method(D_METHOD("cab_activation_auto"), &VehicleController::cab_activation_auto);
        ClassDB::bind_method(D_METHOD("cab_change", "direction"), &VehicleController::cab_change);
        ClassDB::bind_method(
                D_METHOD("main_controller_increase", "step"), &VehicleController::main_controller_increase, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("main_controller_decrease", "step"), &VehicleController::main_controller_decrease, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("second_controller_increase", "step"), &VehicleController::second_controller_increase,
                DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("second_controller_decrease", "step"), &VehicleController::second_controller_decrease,
                DEFVAL(1));
        ClassDB::bind_method(D_METHOD("direction_increase"), &VehicleController::direction_increase);
        ClassDB::bind_method(D_METHOD("direction_decrease"), &VehicleController::direction_decrease);
        ClassDB::bind_method(D_METHOD("radio", "enabled"), &VehicleController::radio);
        ClassDB::bind_method(D_METHOD("radio_channel_set", "channel"), &VehicleController::radio_channel_set);
        ClassDB::bind_method(
                D_METHOD("radio_channel_increase", "step"), &VehicleController::radio_channel_increase, DEFVAL(1));
        ClassDB::bind_method(
                D_METHOD("radio_channel_decrease", "step"), &VehicleController::radio_channel_decrease, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("apply_config"), &VehicleController::apply_config);
        ClassDB::bind_method(D_METHOD("initialize"), &VehicleController::initialize);
        ClassDB::bind_method(D_METHOD("process_components", "delta"), &VehicleController::process_components);
        ClassDB::bind_method(D_METHOD("update_state"), &VehicleController::update_state);
        ClassDB::bind_method(D_METHOD("get_velocity"), &VehicleController::get_velocity);
        ClassDB::bind_method(D_METHOD("get_speed"), &VehicleController::get_speed);
        ClassDB::bind_method(D_METHOD("get_mass_total"), &VehicleController::get_mass_total);
        ClassDB::bind_method(D_METHOD("get_total_distance"), &VehicleController::get_total_distance);
        ClassDB::bind_method(D_METHOD("get_direction"), &VehicleController::get_direction);
        ClassDB::bind_method(D_METHOD("emit_config_changed"), &VehicleController::emit_config_changed);
        ClassDB::bind_method(D_METHOD("apply_configuration"), &VehicleController::apply_configuration);
        ClassDB::bind_method(D_METHOD("is_simulation_ready"), &VehicleController::is_simulation_ready);
        ClassDB::bind_method(D_METHOD("add_component", "component"), &VehicleController::add_component);
        ClassDB::bind_method(D_METHOD("get_component", "type"), &VehicleController::get_component);
        ClassDB::bind_method(
                D_METHOD("find_generic_components", "tag"), &VehicleController::find_generic_components);
        ClassDB::bind_method(D_METHOD("process_movement", "delta"), &VehicleController::process_movement);
        ClassDB::bind_method(D_METHOD("update_location"), &VehicleController::update_location);
        ClassDB::bind_method(
                D_METHOD("update_neighbour", "end", "other", "other_end", "track_distance"),
                &VehicleController::update_neighbour);
        ClassDB::bind_method(D_METHOD("compute_forces", "delta"), &VehicleController::compute_forces);
        ClassDB::bind_method(D_METHOD("compute_movement", "delta"), &VehicleController::compute_movement);
        ClassDB::bind_method(D_METHOD("compute_fast_movement", "delta"), &VehicleController::compute_fast_movement);
        ClassDB::bind_method(D_METHOD("is_physics_active"), &VehicleController::is_physics_active);
        ClassDB::bind_method(
                D_METHOD("couple", "other", "end", "other_end", "coupling_type"), &VehicleController::couple);
        ClassDB::bind_method(D_METHOD("uncouple", "end"), &VehicleController::uncouple);
        ClassDB::bind_method(D_METHOD("is_coupled", "end"), &VehicleController::is_coupled);
        ClassDB::bind_method(D_METHOD("get_coupled_controller", "end"), &VehicleController::get_coupled_controller);
        ClassDB::bind_method(D_METHOD("get_coupled_end", "end"), &VehicleController::get_coupled_end);
        ClassDB::bind_method(D_METHOD("coupler_connect", "where"), &VehicleController::coupler_connect);
        ClassDB::bind_method(D_METHOD("coupler_disconnect", "where"), &VehicleController::coupler_disconnect);
        ClassDB::bind_method(D_METHOD("get_world_transform"), &VehicleController::get_world_transform);
        ClassDB::bind_method(D_METHOD("get_world_position"), &VehicleController::get_world_position);
        ClassDB::bind_method(
                D_METHOD("change_track", "track_name", "track_offset", "track_direction"),
                &VehicleController::change_track);
        ClassDB::bind_method(D_METHOD("get_rid"), &VehicleController::get_rid);
        ClassDB::bind_method(D_METHOD("get_occupied_cab"), &VehicleController::get_occupied_cab);
        ClassDB::bind_method(
                D_METHOD("emit_position_changed_if_needed"), &VehicleController::emit_position_changed_if_needed);
        ClassDB::bind_method(D_METHOD("set_vehicle_rid", "vehicle"), &VehicleController::set_vehicle_rid);

        BIND_PROPERTY(VehicleController, Variant::STRING, train_id);
        BIND_PROPERTY(VehicleController, Variant::STRING, type_name);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, mass);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, max_velocity);
        BIND_PROPERTY(VehicleController, Variant::INT, radio_channel_min, "radio_channel");
        BIND_PROPERTY(VehicleController, Variant::INT, radio_channel_max, "radio_channel");
        /* FIXME: move to TrainPower section? */
        BIND_PROPERTY_W_HINT(VehicleController, Variant::FLOAT, battery_voltage, PROPERTY_HINT_RANGE, "0,500,1");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, category, PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"Train", CATEGORY_TRAIN},
                         {"Road", CATEGORY_ROAD},
                         {"Ship", CATEGORY_SHIP},
                         {"Airplane", CATEGORY_AIRPLANE}}));
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, train_type, PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"Default", TRAIN_TYPE_DEFAULT},
                         {"EZT", TRAIN_TYPE_EZT},
                         {"ET41", TRAIN_TYPE_ET41},
                         {"ET42", TRAIN_TYPE_ET42},
                         {"PseudoDiesel", TRAIN_TYPE_PSEUDODIESEL},
                         {"ET22", TRAIN_TYPE_ET22},
                         {"SN61", TRAIN_TYPE_SN61},
                         {"EP05", TRAIN_TYPE_EP05},
                         {"ET40", TRAIN_TYPE_ET40},
                         {"T181", TRAIN_TYPE_181},
                         {"DMU", TRAIN_TYPE_DMU}}));
        BIND_PROPERTY(VehicleController, Variant::FLOAT, reduced_mass);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, sand_capacity);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, heating_power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, light_power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_length, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_height, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_width, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_drag_coefficient, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_floor_height, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, initial_velocity);
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, driver_type, "", PROPERTY_HINT_ENUM,
                "Nobody,HeadDriver,RearDriver");
        BIND_ENUM_CONSTANT(DRIVER_NOBODY);
        BIND_ENUM_CONSTANT(DRIVER_HEAD);
        BIND_ENUM_CONSTANT(DRIVER_REAR);
        BIND_PROPERTY(VehicleController, Variant::STRING, load_name);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, load_amount);
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_battery_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_ground_relay_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_compartment_lights_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleController, Variant::BOOL, cntrl_automatic_cab_activation, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, cntrl_inactive_cab_flag, "cntrl", PROPERTY_HINT_FLAGS,
                "Emergency Brake,Toggle Mirrors,Raise Second Pantograph,End Of Train Lights,Grant Both Side Permits,"
                "Apply Spring Brake,Release Spring Brake,Reset Direction");

        ADD_SIGNAL(MethodInfo(mover_config_changed_signal));
        ADD_SIGNAL(MethodInfo(mover_initialized_signal));
        ADD_SIGNAL(MethodInfo(power_changed_signal, PropertyInfo(Variant::BOOL, "is_powered")));
        ADD_SIGNAL(MethodInfo(radio_toggled, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(radio_channel_changed, PropertyInfo(Variant::INT, "channel")));
        ADD_SIGNAL(MethodInfo(roof_light_changed, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(cabin_occupied_changed, PropertyInfo(Variant::INT, "cabin_occupied")));
        ADD_SIGNAL(MethodInfo(config_changed));
        ADD_SIGNAL(MethodInfo(position_changed_signal, PropertyInfo(Variant::VECTOR3, "position")));
        ADD_SIGNAL(MethodInfo(consist_changed_signal));
        const String coupling_element_hint = enum_hint({{"Coupler", COUPLING_ELEMENT_COUPLER},
                           {"BrakeHose", COUPLING_ELEMENT_BRAKEHOSE},
                           {"MainHose", COUPLING_ELEMENT_MAINHOSE},
                           {"Control", COUPLING_ELEMENT_CONTROL},
                           {"Gangway", COUPLING_ELEMENT_GANGWAY},
                           {"Heating", COUPLING_ELEMENT_HEATING}});
        ADD_SIGNAL(MethodInfo(
                coupler_attached_signal,
                PropertyInfo(Variant::INT, "element", PROPERTY_HINT_ENUM, coupling_element_hint)));
        ADD_SIGNAL(MethodInfo(
                coupler_detached_signal,
                PropertyInfo(Variant::INT, "element", PROPERTY_HINT_ENUM, coupling_element_hint)));
        ADD_SIGNAL(MethodInfo(
                command_received, PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::NIL, "p1"),
                PropertyInfo(Variant::NIL, "p2")));

        BIND_ENUM_CONSTANT(POWER_SOURCE_NOT_DEFINED);
        BIND_ENUM_CONSTANT(POWER_SOURCE_INTERNAL);
        BIND_ENUM_CONSTANT(POWER_SOURCE_TRANSDUCER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_GENERATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_ACCUMULATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_CURRENTCOLLECTOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_POWERCABLE);
        BIND_ENUM_CONSTANT(POWER_SOURCE_HEATER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_MAIN);

        BIND_ENUM_CONSTANT(POWER_TYPE_NONE);
        BIND_ENUM_CONSTANT(POWER_TYPE_BIO);
        BIND_ENUM_CONSTANT(POWER_TYPE_MECH);
        BIND_ENUM_CONSTANT(POWER_TYPE_ELECTRIC);
        BIND_ENUM_CONSTANT(POWER_TYPE_STEAM);

        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_COUPLER);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_BRAKEHOSE);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_MAINHOSE);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_CONTROL);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_GANGWAY);
        BIND_ENUM_CONSTANT(COUPLING_ELEMENT_HEATING);
        BIND_ENUM_CONSTANT(CATEGORY_TRAIN);
        BIND_ENUM_CONSTANT(CATEGORY_ROAD);
        BIND_ENUM_CONSTANT(CATEGORY_SHIP);
        BIND_ENUM_CONSTANT(CATEGORY_AIRPLANE);

        BIND_ENUM_CONSTANT(TRAIN_TYPE_DEFAULT);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_EZT);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET41);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET42);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_PSEUDODIESEL);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET22);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_SN61);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_EP05);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_ET40);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_181);
        BIND_ENUM_CONSTANT(TRAIN_TYPE_DMU);

        BIND_ENUM_CONSTANT(START_MODE_DISABLED);
        BIND_ENUM_CONSTANT(START_MODE_MANUAL);
        BIND_ENUM_CONSTANT(START_MODE_AUTOMATIC);
        BIND_ENUM_CONSTANT(START_MODE_MANUAL_WITH_AUTO_FALLBACK);
        BIND_ENUM_CONSTANT(START_MODE_CONVERTER);
        BIND_ENUM_CONSTANT(START_MODE_BATTERY);
        BIND_ENUM_CONSTANT(START_MODE_DIRECTION);

        ClassDB::bind_method(D_METHOD("get_tachometer_speed"), &VehicleController::get_tachometer_speed);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "tachometer_speed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_tachometer_speed");
        ClassDB::bind_method(D_METHOD("get_tachometer_speed_jump"), &VehicleController::get_tachometer_speed_jump);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "tachometer_speed_jump", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_tachometer_speed_jump");
        ClassDB::bind_method(D_METHOD("get_tachometer_clock_speed"), &VehicleController::get_tachometer_clock_speed);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "tachometer_clock_speed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_tachometer_clock_speed");
        ClassDB::bind_method(D_METHOD("get_direction_absolute"), &VehicleController::get_direction_absolute);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "direction_absolute", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_direction_absolute");
        ClassDB::bind_method(D_METHOD("get_cabin"), &VehicleController::get_cabin);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "cabin", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabin");
        ClassDB::bind_method(D_METHOD("get_cabin_controleable"), &VehicleController::get_cabin_controleable);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabin_controleable", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabin_controleable");
        ClassDB::bind_method(D_METHOD("get_cabin_occupied"), &VehicleController::get_cabin_occupied);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "cabin_occupied", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cabin_occupied");
        ClassDB::bind_method(D_METHOD("get_live_battery_voltage"), &VehicleController::get_live_battery_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "live_battery_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_live_battery_voltage");
        ClassDB::bind_method(D_METHOD("get_battery_enabled"), &VehicleController::get_battery_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "battery_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_battery_enabled");
        ClassDB::bind_method(D_METHOD("get_radio_enabled"), &VehicleController::get_radio_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "radio_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_radio_enabled");
        ClassDB::bind_method(D_METHOD("get_radio_powered"), &VehicleController::get_radio_powered);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "radio_powered", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_radio_powered");
        ClassDB::bind_method(D_METHOD("get_radio_channel"), &VehicleController::get_radio_channel);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "radio_channel", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_radio_channel");
        ClassDB::bind_method(D_METHOD("get_power24_voltage"), &VehicleController::get_power24_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power24_voltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power24_voltage");
        ClassDB::bind_method(D_METHOD("get_power24_available"), &VehicleController::get_power24_available);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "power24_available", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power24_available");
        ClassDB::bind_method(D_METHOD("get_power110_available"), &VehicleController::get_power110_available);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "power110_available", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power110_available");
        ClassDB::bind_method(D_METHOD("get_current0"), &VehicleController::get_current0);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "current0", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_current0");
        ClassDB::bind_method(D_METHOD("get_current1"), &VehicleController::get_current1);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "current1", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_current1");
        ClassDB::bind_method(D_METHOD("get_current2"), &VehicleController::get_current2);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "current2", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_current2");
        ClassDB::bind_method(D_METHOD("get_relay_novolt"), &VehicleController::get_relay_novolt);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "relay_novolt", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_relay_novolt");
        ClassDB::bind_method(D_METHOD("get_relay_overvoltage"), &VehicleController::get_relay_overvoltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "relay_overvoltage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_relay_overvoltage");
        ClassDB::bind_method(D_METHOD("get_relay_ground"), &VehicleController::get_relay_ground);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "relay_ground", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_relay_ground");
        ClassDB::bind_method(D_METHOD("get_train_damage"), &VehicleController::get_train_damage);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "train_damage", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_train_damage");
        ClassDB::bind_method(D_METHOD("get_controller_second_position"), &VehicleController::get_controller_second_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "controller_second_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_controller_second_position");
        ClassDB::bind_method(D_METHOD("get_controller_main_position"), &VehicleController::get_controller_main_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "controller_main_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_controller_main_position");
        ClassDB::bind_method(D_METHOD("get_controller_joint_position"), &VehicleController::get_controller_joint_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "controller_joint_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_controller_joint_position");
        ClassDB::bind_method(D_METHOD("get_controller_main_actual_position"), &VehicleController::get_controller_main_actual_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "controller_main_actual_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_controller_main_actual_position");
        ClassDB::bind_method(D_METHOD("get_circuit_rlist_size"), &VehicleController::get_circuit_rlist_size);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "circuit_rlist_size", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_circuit_rlist_size");
    }

    void VehicleController::register_command(const String &p_command, const Callable &p_callable) {
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->register_command(train_id, p_command, p_callable);
        }
    }

    void VehicleController::unregister_command(const String &p_command, const Callable &p_callable) {
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->unregister_command(train_id, p_command, p_callable);
        }
    }

    void VehicleController::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        if (p_what == NOTIFICATION_PREDELETE) {
            release();
        }
    }

    /* Applying the vehicle's configuration to the backend: the vehicle's own, then every
     * component's, in registration order. The signal is emitted afterwards and means exactly
     * "the backend now carries this" - it is not how the components are reached, because a
     * component of this vehicle is applied by name here rather than by whoever happens to be
     * connected. */
    void VehicleController::apply_configuration() {
        apply_config();
        for (VehicleComponent *component: components) {
            if (component != nullptr) {
                component->apply_config();
            }
        }
        emit_signal(mover_config_changed_signal);
    }

    /* Registering the vehicle and its commands used to wait for NOTIFICATION_ENTER_TREE. A
     * vehicle is not in a tree any more, so it happens where the vehicle comes into being. */
    /* Registering the vehicle and its own commands. It happens before any component attaches,
     * because a component registers commands too and TrainSystem refuses those of a train it does
     * not know yet. */
    /* A vehicle that is rebuilt keeps its identity - every reference taken to it stays valid -
     * so what it holds is handed back by name rather than by destroying the vehicle. */
    void VehicleController::release() {
        shutdown();
        free_components();
    }

    void VehicleController::attach_to_system() {
        // the vehicle handle is RailVehicle3D's to create; the server hands it here
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->register_train(train_id, this);
        }
        register_command("battery", Callable(this, "battery"));
        register_command("cab_change", Callable(this, "cab_change"));
        register_command("cab_activation", Callable(this, "cab_activation"));
        register_command("cab_activation_auto", Callable(this, "cab_activation_auto"));
        register_command("main_controller_increase", Callable(this, "main_controller_increase"));
        register_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
        register_command("second_controller_increase", Callable(this, "second_controller_increase"));
        register_command("second_controller_decrease", Callable(this, "second_controller_decrease"));
        register_command("direction_increase", Callable(this, "direction_increase"));
        register_command("direction_decrease", Callable(this, "direction_decrease"));
        register_command("radio", Callable(this, "radio"));
        register_command("radio_channel_set", Callable(this, "radio_channel_set"));
        register_command("radio_channel_increase", Callable(this, "radio_channel_increase"));
        register_command("radio_channel_decrease", Callable(this, "radio_channel_decrease"));
        register_command("coupler_connect", Callable(this, "coupler_connect"));
        register_command("coupler_disconnect", Callable(this, "coupler_disconnect"));
    }

    /* The simulation, once every component is attached - _initialize_simulation() pushes the
     * configuration out to all of them (mover_config_changed). */
    void VehicleController::initialize() {
        _initialize_simulation();
        update_state();
        emit_signal(power_changed_signal, prev_is_powered);
        emit_signal(radio_channel_changed, prev_radio_channel);
        emit_signal(roof_light_changed, prev_roof_light_enabled);
    }

    void VehicleController::process_components(const double p_delta) {
        for (VehicleComponent *component: components) {
            component->process(p_delta);
        }
    }

    /// Only marks the state for a rebuild - whoever reads it gets it fresh (see get_state()). The
    /// signals below have to be decided every step though, so they read the live getters directly
    /// rather than through a dictionary that may not be built at all.
    void VehicleController::update_state() {
        if (!is_simulation_ready()) {
            return;
        }
        const bool new_is_powered = get_power24_available() || get_power110_available();
        if (prev_is_powered != new_is_powered) {
            prev_is_powered = new_is_powered; // FIXME: I don't like this
            emit_signal(power_changed_signal, prev_is_powered);
        }

        if (const bool new_radio_enabled = get_radio_enabled() && new_is_powered;
            prev_radio_enabled != new_radio_enabled) {
            prev_radio_enabled = new_radio_enabled; // FIXME: I don't like this
            emit_signal(radio_toggled, new_radio_enabled);
        }

        if (const int new_radio_channel = radio_channel; prev_radio_channel != new_radio_channel) {
            prev_radio_channel = new_radio_channel; // FIXME: I don't like this
            emit_signal(radio_channel_changed, new_radio_channel);
        }

        if (const bool new_roof_light_enabled = lighting != nullptr && lighting->get_roof_light_enabled();
            prev_roof_light_enabled != new_roof_light_enabled) {
            prev_roof_light_enabled = new_roof_light_enabled; // FIXME: I don't like this
            emit_signal(roof_light_changed, new_roof_light_enabled);
        }

        if (const int new_cabin_occupied = get_cabin_occupied(); prev_cabin_occupied != new_cabin_occupied) {
            prev_cabin_occupied = new_cabin_occupied;
            emit_signal(cabin_occupied_changed, new_cabin_occupied);
        }
    }

    void VehicleController::emit_position_changed_if_needed() {
        const Vector3 position = get_world_position();
        if (position.distance_to(last_emitted_position) < 1.0) {
            return;
        }
        last_emitted_position = position;
        emit_signal(position_changed_signal, position);
    }


    int VehicleController::get_radio_channel() const {
        return is_simulation_ready() ? radio_channel : 0;
    }

    void VehicleController::_fill_state_dictionary(Dictionary &p_state) const {
        if (!is_simulation_ready()) {
            return;
        }
        p_state["mass_total"] = get_mass_total();
        p_state["velocity"] = get_velocity();
        p_state["speed"] = get_speed();
        p_state["tachometer_speed"] = get_tachometer_speed();
        p_state["tachometer_speed_jump"] = get_tachometer_speed_jump();
        p_state["tachometer_clock_speed"] = get_tachometer_clock_speed();
        p_state["total_distance"] = get_total_distance();
        p_state["direction"] = get_direction();
        p_state["direction_absolute"] = get_direction_absolute();
        p_state["cabin"] = get_cabin();
        p_state["cabin_controleable"] = get_cabin_controleable();
        p_state["cabin_occupied"] = get_cabin_occupied();
        p_state["battery_enabled"] = get_battery_enabled();
        p_state["battery_voltage"] = get_live_battery_voltage();
        p_state["radio_enabled"] = get_radio_enabled();
        p_state["radio_powered"] = get_radio_powered();
        p_state["radio_channel"] = get_radio_channel();
        p_state["power24_voltage"] = get_power24_voltage();
        p_state["power24_available"] = get_power24_available();
        p_state["power110_available"] = get_power110_available();
        p_state["current0"] = get_current0();
        p_state["current1"] = get_current1();
        p_state["current2"] = get_current2();
        p_state["relay_novolt"] = get_relay_novolt();
        p_state["relay_overvoltage"] = get_relay_overvoltage();
        p_state["relay_ground"] = get_relay_ground();
        p_state["train_damage"] = get_train_damage();
        p_state["controller_second_position"] = get_controller_second_position();
        p_state["controller_main_position"] = get_controller_main_position();
        p_state["controller_joint_position"] = get_controller_joint_position();
        p_state["controller_main_actual_position"] = get_controller_main_actual_position();
        p_state["circuit_rlist_size"] = get_circuit_rlist_size();
    }

    /* The whole vehicle's configuration: its own plus every component's, composed when asked.
     * Unlike the state it is not a view on the backend - the wrapper's own properties and enums
     * are the authoring source of truth, and the backend is configured from them. */
    Dictionary VehicleController::get_config() const {
        Dictionary result;
        _fill_config_dictionary(result);
        for (const VehicleComponent *component: components) {
            component->_fill_config_dictionary(result);
        }
        return result;
    }

    void VehicleController::emit_config_changed() {
        emit_signal(config_changed);
    }

    /// A proxy, not a store: the vehicle keeps no state Dictionary of its own. Every value is
    /// answered by the component that owns it, and this walks them by name for the callers that
    /// still want one - a console, a test, a diagnostic dump. Nothing on the frame path builds it.
    /* The whole vehicle's dump: its own share plus every component's. Expensive on purpose -
     * a console, a test or a diagnostic asks for it, never a per-frame reader. */
    VehicleComponent *VehicleController::get_component(const VehicleComponentType::Type p_type) const {
        for (VehicleComponent *component: components) {
            if (component->get_component_type() == p_type) {
                return component;
            }
        }
        return nullptr;
    }

    TypedArray<VehicleComponent> VehicleController::find_generic_components(const StringName &p_tag) const {
        TypedArray<VehicleComponent> found;
        for (VehicleComponent *component: components) {
            if (component->get_component_type() == VehicleComponentType::COMPONENT_GENERIC
                && component->get_component_tag() == p_tag) {
                found.push_back(component);
            }
        }
        return found;
    }

    /* Attaching a component is a complete operation: it joins the vehicle and, when the vehicle
     * is already running, its configuration is written to the backend and announced there and
     * then. A component added to a built vehicle - a modder's, or one a test adds - must not
     * leave the vehicle describing geometry it does not have. */
    void VehicleController::add_component(VehicleComponent *p_component) {
        ERR_FAIL_NULL(p_component);
        p_component->attach(this);
        if (is_simulation_ready()) {
            p_component->apply_config();
        }
    }

    /* Every component goes with the vehicle; nothing outside it holds one. */
    void VehicleController::shutdown() {
        unregister_command("battery", Callable(this, "battery"));
        unregister_command("cab_change", Callable(this, "cab_change"));
        unregister_command("cab_activation", Callable(this, "cab_activation"));
        unregister_command("cab_activation_auto", Callable(this, "cab_activation_auto"));
        unregister_command("main_controller_increase", Callable(this, "main_controller_increase"));
        unregister_command("main_controller_decrease", Callable(this, "main_controller_decrease"));
        unregister_command("second_controller_increase", Callable(this, "second_controller_increase"));
        unregister_command("second_controller_decrease", Callable(this, "second_controller_decrease"));
        unregister_command("direction_increase", Callable(this, "direction_increase"));
        unregister_command("direction_decrease", Callable(this, "direction_decrease"));
        unregister_command("radio", Callable(this, "radio"));
        unregister_command("radio_channel_set", Callable(this, "radio_channel_set"));
        unregister_command("radio_channel_increase", Callable(this, "radio_channel_increase"));
        unregister_command("radio_channel_decrease", Callable(this, "radio_channel_decrease"));
        unregister_command("coupler_connect", Callable(this, "coupler_connect"));
        unregister_command("coupler_disconnect", Callable(this, "coupler_disconnect"));
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->unregister_train(train_id);
        }
        // the handle belongs to RailVehicle3D, which frees it with itself
        rid = RID();
    }

    void VehicleController::free_components() {
        const Vector<VehicleComponent *> owned = components;
        components.clear();
        lighting = nullptr;
        for (VehicleComponent *component: owned) {
            component->detach();
            memdelete(component);
        }
    }

    void VehicleController::register_component(VehicleComponent *p_component) {
        components.push_back(p_component);
        _component_attached(p_component);
        if (VehicleLighting *component_lighting = Object::cast_to<VehicleLighting>(p_component);
            component_lighting != nullptr) {
            lighting = component_lighting;
        }
    }

    void VehicleController::unregister_component(VehicleComponent *p_component) {
        _component_detached(p_component);
        components.erase(p_component);
        if (static_cast<VehicleComponent *>(lighting) == p_component) {
            lighting = nullptr;
        }
    }

    Dictionary VehicleController::get_state() {
        Dictionary result;
        _fill_state_dictionary(result);
        for (VehicleComponent *component: components) {
            if (component->get_enabled()) {
                component->_fill_state_dictionary(result);
            }
        }
        return result;
    }

    void
    VehicleController::change_track(const String &p_track_name, const float p_track_offset, const int p_track_direction) {
        UtilityFunctions::push_warning(
                vformat("VehicleController::change_track() is managed by RailVehicle3D now: %s / %.3f / %d", p_track_name,
                        p_track_offset, p_track_direction));
    }

    Vector3 VehicleController::get_world_position() const {
        return get_world_transform().get_origin();
    }

    Transform3D VehicleController::get_world_transform() const {
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr || !rid.is_valid()) {
            return Transform3D();
        }
        return server->vehicle_get_transform(rid);
    }

    void VehicleController::set_vehicle_rid(const RID &p_vehicle_rid) {
        rid = p_vehicle_rid;
    }

    RID VehicleController::get_rid() const {
        return rid;
    }

    void VehicleController::command_executed(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        ++command_serial;
        update_state();
        emit_signal(command_received, p_command, p_p1, p_p2);
    }

    uint64_t VehicleController::get_command_serial() const {
        return command_serial;
    }

    void VehicleController::broadcast_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->broadcast_command(p_command, p_p1, p_p2);
        }
    }

    Variant VehicleController::send_command(const StringName &p_command, const Variant &p_p1, const Variant &p_p2) const {
        TrainSystem *system = TrainSystem::get_instance();
        return system != nullptr ? system->send_command(train_id, String(p_command), p_p1, p_p2) : Variant();
    }

    void VehicleController::set_driver_type(const DriverType p_value) {
        driver_type = p_value;
    }

    VehicleController::DriverType VehicleController::get_driver_type() const {
        return driver_type;
    }

    /* The backend counts the occupied cab as +1 for the front one and -1 for the rear
     * (DynObj.cpp:1812-1825); nobody aboard is 0, and that is what keeps an unmanned vehicle out
     * of the physics. */
    int VehicleController::get_occupied_cab() const {
        switch (driver_type) {
            case DRIVER_HEAD:
                return 1;
            case DRIVER_REAR:
                return -1;
            default:
                return 0;
        }
    }

    void VehicleController::radio_channel_increase(const int p_step) {
        const int step = p_step > 0 ? p_step : 1;
        radio_channel = Math::clamp(radio_channel + step, radio_channel_min, radio_channel_max);
    }

    void VehicleController::radio_channel_decrease(const int p_step) {
        const int step = (p_step != 0) ? p_step : 1;
        radio_channel = Math::clamp(radio_channel - step, radio_channel_min, radio_channel_max);
    }

    void VehicleController::radio_channel_set(const int p_channel) {
        radio_channel = Math::clamp(p_channel, radio_channel_min, radio_channel_max);
    }

} // namespace godot
