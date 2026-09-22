#include "../brakes/VehicleBrake.hpp"
#include "../core/VehicleController.hpp"
#include "../core/utils.hpp"
#include <algorithm>
#include <cmath>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleBrake::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, valve_type, "valve", PROPERTY_HINT_ENUM,
                "NoValve,W,W_Lu_VI,W_Lu_L,W_Lu_XR,K,Kg,Kp,Kss,Kkg,Kkp,Kks,Hikg1,Hikss,Hikp1,KE,SW,EStED,NESt3,ESt3,LSt,"
                "ESt4,ESt3AL2,EP1,EP2,M483,CV1_L_TR,CV1,CV1_R,Other")
        BIND_PROPERTY(VehicleBrake, Variant::INT, friction_elements_per_axle);
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, brake_force_max, "brake_force");
        BIND_PROPERTY(VehicleBrake, Variant::INT, est_valve_size, "est_valve");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, brake_force_traction, "brake_force");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, max_cylinder_pressure);
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, max_aux_pressure);
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, max_tare_pressure);
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, max_medium_pressure);
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, max_antislip_pressure);
        BIND_PROPERTY(VehicleBrake, Variant::INT, cylinder_count, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cylinder_radius, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cylinder_distance, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cylinder_spring_force, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, piston_stroke_adjuster_resistance, "piston_stroke");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cylinder_gear_ratio, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cylinder_gear_ratio_low, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cylinder_gear_ratio_high, "cylinder");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, pipe_pressure_min, "pipe");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, pipe_pressure_max, "pipe");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, tank_volume_main, "tank");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, tank_volume_aux, "tank");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_cab_a_min_pressure, "compressor/cab_a");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_cab_a_max_pressure, "compressor/cab_a");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_cab_b_min_pressure, "compressor/cab_b");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_cab_b_max_pressure, "compressor/cab_b");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_speed, "compressor");
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, compressor_power, "compressor", PROPERTY_HINT_ENUM,
                "Main,Unused,Converter,Engine,Coupler1,Coupler2");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, rig_effectiveness);
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, brake_method, "brake", PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"P10-Bgu", BRAKE_METHOD_P10_BGU},
                         {"P10-Bg", BRAKE_METHOD_P10_BG},
                         {"Disk1", BRAKE_METHOD_D1},
                         {"Disk2", BRAKE_METHOD_D2},
                         {"FR513", BRAKE_METHOD_FR513},
                         {"Cosid", BRAKE_METHOD_COSID},
                         {"P10yBg", BRAKE_METHOD_P10Y_BG},
                         {"P10yBgu", BRAKE_METHOD_P10Y_BGU},
                         {"FR510", BRAKE_METHOD_FR510},
                         {"Disk1+Mg", BRAKE_METHOD_D1MG}}));
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, rapid_transfer, "rapid");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, rapid_switching_speed, "rapid");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, air_leak_multiplier)
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, compressor_tank_valve_active, "compressor")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_lower_emergency_closing_pressure, "compressor")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_higher_emergency_closing_pressure, "compressor")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, main_pipe_blocking_pressure, "main_pipe")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, main_pipe_unblocking_pressure, "main_pipe")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, main_pipe_minimum_unblocking_handle_position, "main_pipe")
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, main_pipe_emergency_cuts_off_handle, "main_pipe")
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleBrake, Variant::ARRAY, brake_pressure_table, PROPERTY_HINT_TYPE_STRING, "BrakePressureTableItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleBrake, Variant::ARRAY, compressor_list, PROPERTY_HINT_TYPE_STRING, "CompressorListItem");
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, compressor_emergency_valve_area, "compressor")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, universal_brake_button_1, "universal_brake_button", PROPERTY_HINT_FLAGS,
                "Releaser,Bridge Emergency Valve,High Pressure Impulse,Assimilation,Anti-Skid Brake")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, universal_brake_button_2, "universal_brake_button", PROPERTY_HINT_FLAGS,
                "Releaser,Bridge Emergency Valve,High Pressure Impulse,Assimilation,Anti-Skid Brake")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, universal_brake_button_3, "universal_brake_button", PROPERTY_HINT_FLAGS,
                "Releaser,Bridge Emergency Valve,High Pressure Impulse,Assimilation,Anti-Skid Brake")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_brake_system, "cntrl", PROPERTY_HINT_ENUM,
                "Individual,Pneumatic,ElectroPneumatic")
        BIND_PROPERTY(VehicleBrake, Variant::INT, cntrl_brake_ctrl_position_count, "cntrl")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_brake_delays, "cntrl", PROPERTY_HINT_ENUM,
                "G:1,P:2,R:4,GP:3,PR:6,GPR:7,PR+Mg:14,GPR+Mg:15")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cntrl_brake_delay_1, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cntrl_brake_delay_2, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cntrl_brake_delay_3, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cntrl_brake_delay_4, "cntrl")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_brake_op_modes, "cntrl", PROPERTY_HINT_ENUM, "PN:3,PNEPMED:15")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_brake_handle_type, "cntrl", PROPERTY_HINT_ENUM,
                "NoHandle,Westinghouse,FV4a,M394,M254,FVE408,FVel6,D2,Knorr,FD1,BS2,testH,St113,MHZ_P,MHZ_T,MHZ_EN57,"
                "MHZ_K5P,MHZ_K8P,MHZ_6P")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_anti_skid_brake_type, "cntrl", PROPERTY_HINT_ENUM,
                "None,Manual,Automatic")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_local_brake_type, "cntrl", PROPERTY_HINT_ENUM,
                "None,Manual,Pneumatic,Hydraulic")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_local_brake_handle_type, "cntrl", PROPERTY_HINT_ENUM,
                "NoHandle,Westinghouse,FV4a,M394,M254,FVE408,FVel6,D2,Knorr,FD1,BS2,testH,St113,MHZ_P,MHZ_T,MHZ_EN57,"
                "MHZ_K5P,MHZ_K8P,MHZ_6P")
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, cntrl_manual_brake_present, "cntrl")
        BIND_PROPERTY_W_HINT(
                VehicleBrake, Variant::INT, cntrl_dynamic_brake_type, "cntrl", PROPERTY_HINT_ENUM,
                "None:0,Passive:1,Switch:2,Reversal:4,Automatic:8")
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, cntrl_local_brake_traxx, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, cntrl_release_parking_by_spring_brake, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, cntrl_release_parking_by_spring_brake_when_door_open, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::BOOL, cntrl_spring_brake_cuts_off_drive, "cntrl")
        BIND_PROPERTY(VehicleBrake, Variant::FLOAT, cntrl_spring_brake_drive_emergency_velocity, "cntrl")

        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_NO_HANDLE);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_WESTINGHOUSE);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_FV4A);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_M394);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_M254);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_FVE408);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_FVEL6);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_D2);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_KNORR);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_FD1);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_BS2);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_TESTH);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_ST113);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_MHZ_P);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_MHZ_T);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_MHZ_EN57);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_MHZ_K5P);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_MHZ_K8P);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_TYPE_MHZ_6P);

        BIND_ENUM_CONSTANT(LOCAL_BRAKE_TYPE_NONE);
        BIND_ENUM_CONSTANT(LOCAL_BRAKE_TYPE_MANUAL);
        BIND_ENUM_CONSTANT(LOCAL_BRAKE_TYPE_PNEUMATIC);
        BIND_ENUM_CONSTANT(LOCAL_BRAKE_TYPE_HYDRAULIC);

        BIND_ENUM_CONSTANT(ANTI_SKID_BRAKE_NONE);
        BIND_ENUM_CONSTANT(ANTI_SKID_BRAKE_MANUAL);
        BIND_ENUM_CONSTANT(ANTI_SKID_BRAKE_AUTOMATIC);

        BIND_ENUM_CONSTANT(DYNAMIC_BRAKE_NONE);
        BIND_ENUM_CONSTANT(DYNAMIC_BRAKE_PASSIVE);
        BIND_ENUM_CONSTANT(DYNAMIC_BRAKE_SWITCH);
        BIND_ENUM_CONSTANT(DYNAMIC_BRAKE_REVERSAL);
        BIND_ENUM_CONSTANT(DYNAMIC_BRAKE_AUTOMATIC);

        BIND_ENUM_CONSTANT(BRAKE_DELAY_G);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_P);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_R);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_GP);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_PR);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_GPR);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_PR_MG);
        BIND_ENUM_CONSTANT(BRAKE_DELAY_GPR_MG);

        BIND_ENUM_CONSTANT(BRAKE_OP_MODE_PN);
        BIND_ENUM_CONSTANT(BRAKE_OP_MODE_PNEPMED);

        BIND_ENUM_CONSTANT(BRAKE_SYSTEM_INDIVIDUAL);
        BIND_ENUM_CONSTANT(BRAKE_SYSTEM_PNEUMATIC);
        BIND_ENUM_CONSTANT(BRAKE_SYSTEM_ELECTRO_PNEUMATIC);

        BIND_ENUM_CONSTANT(COMPRESSOR_POWER_MAIN);
        BIND_ENUM_CONSTANT(COMPRESSOR_POWER_UNUSED);
        BIND_ENUM_CONSTANT(COMPRESSOR_POWER_CONVERTER);
        BIND_ENUM_CONSTANT(COMPRESSOR_POWER_ENGINE);
        BIND_ENUM_CONSTANT(COMPRESSOR_POWER_COUPLER1);
        BIND_ENUM_CONSTANT(COMPRESSOR_POWER_COUPLER2);

        BIND_ENUM_CONSTANT(BRAKE_HANDLE_POSITION_MIN);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_POSITION_MAX);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_POSITION_DRIVE);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_POSITION_FULL);
        BIND_ENUM_CONSTANT(BRAKE_HANDLE_POSITION_EMERGENCY);

        BIND_ENUM_CONSTANT(BRAKE_VALVE_NO_VALVE);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_W);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_W_LU_VI);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_W_LU_L);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_W_LU_XR);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_K);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KG);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KP);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KSS);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KKG);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KKP);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KKS);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_HIKG1);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_HIKSS);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_HIKP1);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_KE);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_SW);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_ESTED);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_NEST3);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_EST3);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_LST);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_EST4);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_EST3AL2);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_EP1);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_EP2);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_M483);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_CV1_L_TR);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_CV1);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_CV1_R);
        BIND_ENUM_CONSTANT(BRAKE_VALVE_OTHER);

        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10_BGU);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10_BG);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_D1);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_D2);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_FR513);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_COSID);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10Y_BG);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10Y_BGU);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_D1MG);

        ClassDB::bind_method(D_METHOD("brake_releaser", "enabled"), &VehicleBrake::brake_releaser);
        ClassDB::bind_method(D_METHOD("brake_level_set", "level"), &VehicleBrake::brake_level_set);
        ClassDB::bind_method(D_METHOD("brake_level_set_position", "position"), &VehicleBrake::brake_level_set_position);
        ClassDB::bind_method(
                D_METHOD("brake_level_set_position_str", "position"), &VehicleBrake::brake_level_set_position_str);
        ClassDB::bind_method(D_METHOD("brake_level_increase"), &VehicleBrake::brake_level_increase);
        ClassDB::bind_method(D_METHOD("brake_level_decrease"), &VehicleBrake::brake_level_decrease);
        ClassDB::bind_method(D_METHOD("local_brake_set", "level"), &VehicleBrake::local_brake_set);
        ClassDB::bind_method(D_METHOD("local_brake_increase"), &VehicleBrake::local_brake_increase);
        ClassDB::bind_method(D_METHOD("local_brake_decrease"), &VehicleBrake::local_brake_decrease);
        ClassDB::bind_method(D_METHOD("manual_brake_increase"), &VehicleBrake::manual_brake_increase);
        ClassDB::bind_method(D_METHOD("manual_brake_decrease"), &VehicleBrake::manual_brake_decrease);
        ClassDB::bind_method(D_METHOD("auto_rewident", "brake_delay"), &VehicleBrake::auto_rewident);
        ClassDB::bind_method(D_METHOD("brake_level_charging", "active"), &VehicleBrake::brake_level_charging);
        ClassDB::bind_method(D_METHOD("alarm_chain", "pulled"), &VehicleBrake::alarm_chain);

        ClassDB::bind_method(D_METHOD("get_alarm_chain_pulled"), &VehicleBrake::get_alarm_chain_pulled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "alarm_chain_pulled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_alarm_chain_pulled");
        ClassDB::bind_method(D_METHOD("get_air_pressure"), &VehicleBrake::get_air_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "air_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_air_pressure");
        ClassDB::bind_method(D_METHOD("get_loco_pressure"), &VehicleBrake::get_loco_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "loco_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_loco_pressure");
        ClassDB::bind_method(D_METHOD("get_pipe_brake_pressure"), &VehicleBrake::get_pipe_brake_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "pipe_brake_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_pipe_brake_pressure");
        ClassDB::bind_method(D_METHOD("get_pipe_pressure"), &VehicleBrake::get_pipe_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "pipe_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_pipe_pressure");
        ClassDB::bind_method(D_METHOD("get_feed_pipe_pressure"), &VehicleBrake::get_feed_pipe_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "feed_pipe_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_feed_pipe_pressure");
        ClassDB::bind_method(D_METHOD("get_tank_volume"), &VehicleBrake::get_tank_volume);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "tank_volume", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_tank_volume");
        ClassDB::bind_method(D_METHOD("get_compressor_pressure"), &VehicleBrake::get_compressor_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "compressor_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_compressor_pressure");
        ClassDB::bind_method(D_METHOD("get_controller_position"), &VehicleBrake::get_controller_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "controller_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_controller_position");
        ClassDB::bind_method(D_METHOD("get_controller_position_normalized"), &VehicleBrake::get_controller_position_normalized);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "controller_position_normalized", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_controller_position_normalized");
        ClassDB::bind_method(D_METHOD("get_local_position_normalized"), &VehicleBrake::get_local_position_normalized);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "local_position_normalized", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_local_position_normalized");
        ClassDB::bind_method(D_METHOD("get_manual_position"), &VehicleBrake::get_manual_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "manual_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_manual_position");
        ClassDB::bind_method(D_METHOD("get_unit_force"), &VehicleBrake::get_unit_force);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "unit_force", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_unit_force");
        ClassDB::bind_method(D_METHOD("get_force_ratio"), &VehicleBrake::get_force_ratio);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "force_ratio", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_force_ratio");
        ClassDB::bind_method(D_METHOD("get_emergency_valve_flow"), &VehicleBrake::get_emergency_valve_flow);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "emergency_valve_flow", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_emergency_valve_flow");
        ClassDB::bind_method(D_METHOD("get_main_valve_flow"), &VehicleBrake::get_main_valve_flow);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "main_valve_flow", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_main_valve_flow");
        ClassDB::bind_method(D_METHOD("get_local_valve_flow"), &VehicleBrake::get_local_valve_flow);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "local_valve_flow", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_local_valve_flow");
        ClassDB::bind_method(D_METHOD("get_loco_pressure_fall_rate"), &VehicleBrake::get_loco_pressure_fall_rate);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "loco_pressure_fall_rate", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_loco_pressure_fall_rate");
        ClassDB::bind_method(D_METHOD("get_loco_pressure_rise_rate"), &VehicleBrake::get_loco_pressure_rise_rate);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "loco_pressure_rise_rate", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_loco_pressure_rise_rate");
        ClassDB::bind_method(D_METHOD("get_control_pressure"), &VehicleBrake::get_control_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "control_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_control_pressure");
        ClassDB::bind_method(D_METHOD("get_local_aeim_position"), &VehicleBrake::get_local_aeim_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "local_aeim_position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_local_aeim_position");
        ClassDB::bind_method(D_METHOD("get_edb_cylinder_pressure"), &VehicleBrake::get_edb_cylinder_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "edb_cylinder_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_edb_cylinder_pressure");
        ClassDB::bind_method(D_METHOD("get_releaser_active"), &VehicleBrake::get_releaser_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "releaser_active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_releaser_active");
    }

    void VehicleBrake::_register_commands() {
        register_command("brake_releaser", Callable(this, "brake_releaser"));
        register_command("brake_level_set", Callable(this, "brake_level_set"));
        register_command("brake_level_set_position", Callable(this, "brake_level_set_position_str"));
        register_command("brake_level_increase", Callable(this, "brake_level_increase"));
        register_command("brake_level_decrease", Callable(this, "brake_level_decrease"));
        register_command("local_brake_set", Callable(this, "local_brake_set"));
        register_command("local_brake_increase", Callable(this, "local_brake_increase"));
        register_command("local_brake_decrease", Callable(this, "local_brake_decrease"));
        register_command("manual_brake_increase", Callable(this, "manual_brake_increase"));
        register_command("manual_brake_decrease", Callable(this, "manual_brake_decrease"));
        register_command("auto_rewident", Callable(this, "auto_rewident"));
        register_command("brake_level_charging", Callable(this, "brake_level_charging"));
        register_command("alarm_chain", Callable(this, "alarm_chain"));
    }

    void VehicleBrake::_unregister_commands() {
        unregister_command("brake_releaser", Callable(this, "brake_releaser"));
        unregister_command("brake_level_set", Callable(this, "brake_level_set"));
        unregister_command("brake_level_set_position", Callable(this, "brake_level_set_position_str"));
        unregister_command("brake_level_increase", Callable(this, "brake_level_increase"));
        unregister_command("brake_level_decrease", Callable(this, "brake_level_decrease"));
        unregister_command("local_brake_set", Callable(this, "local_brake_set"));
        unregister_command("local_brake_increase", Callable(this, "local_brake_increase"));
        unregister_command("local_brake_decrease", Callable(this, "local_brake_decrease"));
        unregister_command("manual_brake_increase", Callable(this, "manual_brake_increase"));
        unregister_command("manual_brake_decrease", Callable(this, "manual_brake_decrease"));
        unregister_command("auto_rewident", Callable(this, "auto_rewident"));
        unregister_command("brake_level_charging", Callable(this, "brake_level_charging"));
        unregister_command("alarm_chain", Callable(this, "alarm_chain"));
    }
} // namespace godot
