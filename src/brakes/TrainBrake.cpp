#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include "../core/utils.hpp"
#include <algorithm>
#include <cmath>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainBrake::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, valve_type, "valve", PROPERTY_HINT_ENUM,
                "NoValve,W,W_Lu_VI,W_Lu_L,W_Lu_XR,K,Kg,Kp,Kss,Kkg,Kkp,Kks,Hikg1,Hikss,Hikp1,KE,SW,EStED,NESt3,ESt3,LSt,"
                "ESt4,ESt3AL2,EP1,EP2,M483,CV1_L_TR,CV1,CV1_R,Other")
        BIND_PROPERTY(TrainBrake, Variant::INT, friction_elements_per_axle);
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, brake_force_max, "brake_force");
        BIND_PROPERTY(TrainBrake, Variant::INT, est_valve_size, "est_valve");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, brake_force_traction, "brake_force");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, max_cylinder_pressure);
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, max_aux_pressure);
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, max_tare_pressure);
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, max_medium_pressure);
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, max_antislip_pressure);
        BIND_PROPERTY(TrainBrake, Variant::INT, cylinder_count, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cylinder_radius, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cylinder_distance, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cylinder_spring_force, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, piston_stroke_adjuster_resistance, "piston_stroke");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cylinder_gear_ratio, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cylinder_gear_ratio_low, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cylinder_gear_ratio_high, "cylinder");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, pipe_pressure_min, "pipe");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, pipe_pressure_max, "pipe");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, tank_volume_main, "tank");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, tank_volume_aux, "tank");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_cab_a_min_pressure, "compressor/cab_a");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_cab_a_max_pressure, "compressor/cab_a");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_cab_b_min_pressure, "compressor/cab_b");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_cab_b_max_pressure, "compressor/cab_b");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_speed, "compressor");
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, compressor_power, "compressor", PROPERTY_HINT_ENUM,
                "Main,Unused,Converter,Engine,Coupler1,Coupler2");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, rig_effectiveness);
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, brake_method, "brake", PROPERTY_HINT_ENUM,
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
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, rapid_transfer, "rapid");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, rapid_switching_speed, "rapid");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, air_leak_multiplier)
        BIND_PROPERTY(TrainBrake, Variant::BOOL, compressor_tank_valve_active, "compressor")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_lower_emergency_closing_pressure, "compressor")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_higher_emergency_closing_pressure, "compressor")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, main_pipe_blocking_pressure, "main_pipe")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, main_pipe_unblocking_pressure, "main_pipe")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, main_pipe_minimum_unblocking_handle_position, "main_pipe")
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainBrake, Variant::ARRAY, brake_pressure_table, PROPERTY_HINT_TYPE_STRING, "BrakePressureTableItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainBrake, Variant::ARRAY, compressor_list, PROPERTY_HINT_TYPE_STRING, "CompressorListItem");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_emergency_valve_area, "compressor")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, universal_brake_button_1, "universal_brake_button", PROPERTY_HINT_FLAGS,
                "Releaser,Bridge Emergency Valve,High Pressure Impulse,Assimilation,Anti-Skid Brake")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, universal_brake_button_2, "universal_brake_button", PROPERTY_HINT_FLAGS,
                "Releaser,Bridge Emergency Valve,High Pressure Impulse,Assimilation,Anti-Skid Brake")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, universal_brake_button_3, "universal_brake_button", PROPERTY_HINT_FLAGS,
                "Releaser,Bridge Emergency Valve,High Pressure Impulse,Assimilation,Anti-Skid Brake")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_brake_system, "cntrl", PROPERTY_HINT_ENUM,
                "Individual,Pneumatic,ElectroPneumatic")
        BIND_PROPERTY(TrainBrake, Variant::INT, cntrl_brake_ctrl_position_count, "cntrl")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_brake_delays, "cntrl", PROPERTY_HINT_ENUM,
                "G:1,P:2,R:4,GP:3,PR:6,GPR:7,PR+Mg:14,GPR+Mg:15")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cntrl_brake_delay_1, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cntrl_brake_delay_2, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cntrl_brake_delay_3, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cntrl_brake_delay_4, "cntrl")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_brake_op_modes, "cntrl", PROPERTY_HINT_ENUM, "PN:3,PNEPMED:15")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_brake_handle_type, "cntrl", PROPERTY_HINT_ENUM,
                "NoHandle,Westinghouse,FV4a,M394,M254,FVE408,FVel6,D2,Knorr,FD1,BS2,testH,St113,MHZ_P,MHZ_T,MHZ_EN57,"
                "MHZ_K5P,MHZ_K8P,MHZ_6P")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_anti_skid_brake_type, "cntrl", PROPERTY_HINT_ENUM,
                "None,Manual,Automatic")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_local_brake_type, "cntrl", PROPERTY_HINT_ENUM,
                "None,Manual,Pneumatic,Hydraulic")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_local_brake_handle_type, "cntrl", PROPERTY_HINT_ENUM,
                "NoHandle,Westinghouse,FV4a,M394,M254,FVE408,FVel6,D2,Knorr,FD1,BS2,testH,St113,MHZ_P,MHZ_T,MHZ_EN57,"
                "MHZ_K5P,MHZ_K8P,MHZ_6P")
        BIND_PROPERTY(TrainBrake, Variant::BOOL, cntrl_manual_brake_present, "cntrl")
        BIND_PROPERTY_W_HINT(
                TrainBrake, Variant::INT, cntrl_dynamic_brake_type, "cntrl", PROPERTY_HINT_ENUM,
                "None:0,Passive:1,Switch:2,Reversal:4,Automatic:8")
        BIND_PROPERTY(TrainBrake, Variant::BOOL, cntrl_local_brake_traxx, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::BOOL, cntrl_release_parking_by_spring_brake, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::BOOL, cntrl_release_parking_by_spring_brake_when_door_open, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::BOOL, cntrl_spring_brake_cuts_off_drive, "cntrl")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, cntrl_spring_brake_drive_emergency_velocity, "cntrl")

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

        ClassDB::bind_method(D_METHOD("brake_releaser", "enabled"), &TrainBrake::brake_releaser);
        ClassDB::bind_method(D_METHOD("brake_level_set", "level"), &TrainBrake::brake_level_set);
        ClassDB::bind_method(D_METHOD("brake_level_set_position", "position"), &TrainBrake::brake_level_set_position);
        ClassDB::bind_method(
                D_METHOD("brake_level_set_position_str", "position"), &TrainBrake::brake_level_set_position_str);
        ClassDB::bind_method(D_METHOD("brake_level_increase"), &TrainBrake::brake_level_increase);
        ClassDB::bind_method(D_METHOD("brake_level_decrease"), &TrainBrake::brake_level_decrease);
        ClassDB::bind_method(D_METHOD("local_brake_set", "level"), &TrainBrake::local_brake_set);
        ClassDB::bind_method(D_METHOD("local_brake_increase"), &TrainBrake::local_brake_increase);
        ClassDB::bind_method(D_METHOD("local_brake_decrease"), &TrainBrake::local_brake_decrease);
        ClassDB::bind_method(D_METHOD("alarm_chain", "pulled"), &TrainBrake::alarm_chain);
    }

    void TrainBrake::_register_commands() {
        register_command("brake_releaser", Callable(this, "brake_releaser"));
        register_command("brake_level_set", Callable(this, "brake_level_set"));
        register_command("brake_level_set_position", Callable(this, "brake_level_set_position_str"));
        register_command("brake_level_increase", Callable(this, "brake_level_increase"));
        register_command("brake_level_decrease", Callable(this, "brake_level_decrease"));
        register_command("local_brake_set", Callable(this, "local_brake_set"));
        register_command("local_brake_increase", Callable(this, "local_brake_increase"));
        register_command("local_brake_decrease", Callable(this, "local_brake_decrease"));
        register_command("alarm_chain", Callable(this, "alarm_chain"));
    }

    void TrainBrake::_unregister_commands() {
        unregister_command("brake_releaser", Callable(this, "brake_releaser"));
        unregister_command("brake_level_set", Callable(this, "brake_level_set"));
        unregister_command("brake_level_set_position", Callable(this, "brake_level_set_position_str"));
        unregister_command("brake_level_increase", Callable(this, "brake_level_increase"));
        unregister_command("brake_level_decrease", Callable(this, "brake_level_decrease"));
        unregister_command("local_brake_set", Callable(this, "local_brake_set"));
        unregister_command("local_brake_increase", Callable(this, "local_brake_increase"));
        unregister_command("local_brake_decrease", Callable(this, "local_brake_decrease"));
        unregister_command("alarm_chain", Callable(this, "alarm_chain"));
    }

    void TrainBrake::alarm_chain(const bool p_pulled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        // Train.cpp:1839-1872 (OnCommand_alarmchaintoggle/enable/disable) ->
        // AlarmChainSwitch(State) - manual emergency brake pull cord.
        mover->AlarmChainSwitch(p_pulled);
    }

    void TrainBrake::brake_releaser(const bool p_pressed) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->BrakeReleaser(p_pressed ? 1 : 0);
    }

    void TrainBrake::brake_level_set(const double p_level) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        const double level = CLAMP(p_level, 0.0, 1.0);
        const double brake_controller_min = mover->Handle->GetPos(bh_MIN);
        const double brake_controller_max = mover->Handle->GetPos(bh_MAX);
        const double brake_controller_pos =
                brake_controller_min + (level * (brake_controller_max - brake_controller_min));
        mover->BrakeLevelSet(brake_controller_pos);
    }

    void TrainBrake::brake_level_set_position(const BrakeHandlePosition p_position) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        if (const std::unordered_map<BrakeHandlePosition, int>::const_iterator it =
                    brake_handle_position_map.find(p_position);
            it != brake_handle_position_map.end()) {
            mover->BrakeLevelSet(mover->Handle->GetPos(it->second));
        } else {
            log_error("Unhandled brake level position: " + String::num(static_cast<int>(p_position)));
        }
    }

    void TrainBrake::brake_level_set_position_str(const String &p_position) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        const std::unordered_map<std::string, int>::const_iterator it =
                brake_handle_position_string_map.find(std::string(p_position.utf8()));
        if (it != brake_handle_position_string_map.end()) {
            mover->BrakeLevelSet(mover->Handle->GetPos(it->second));
        } else {
            log_error("Unhandled brake level position: " + p_position);
        }
    }

    void TrainBrake::brake_level_increase() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->IncBrakeLevel();
    }

    void TrainBrake::brake_level_decrease() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->DecBrakeLevel();
    }

    // Original engine: "localbrake:"/ggLocalBrake (Train.cpp:10026) is the independent/loco
    // brake handle - a draggable gauge like mainctrl/brakectrl, not a passive display. Real
    // input is OnCommand_independentbrakeincrease/decrease (Train.cpp:1447-1524), bound by
    // default to num_1/num_7 (eu07_input-keyboard.ini) - there was previously no equivalent
    // command in this wrapper at all, so the handle could never move.
    void TrainBrake::local_brake_set(const double p_level) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        // LocalBrakePosA is already normalized 0..1 in the mover (unlike the main brake's
        // arbitrary Handle-position units), so no range conversion is needed here.
        mover->LocalBrakePosA = CLAMP(p_level, 0.0, 1.0);
    }

    void TrainBrake::local_brake_increase() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        // One notch per call, mirroring this wrapper's main_controller_increase(step=1)
        // convention for a single cab-click/command invocation.
        mover->IncLocalBrakeLevel(1);
    }

    void TrainBrake::local_brake_decrease() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        mover->DecLocalBrakeLevel(1);
    }

    void TrainBrake::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        // Hardware/setup facts - change only when the vehicle's brake config is (re)applied, not
        // every tick, so they belong here rather than in _do_fetch_state_from_mover. Read from
        // TrainBrake's own already-bound properties (the authoring source of truth) rather than
        // re-deriving from mover internals a second time.
        p_config["brake_ep_enabled"] = p_mover->BrakeSystem == TBrakeSystem::ElectroPneumatic;
        p_config["brake_handle_type"] = get_cntrl_brake_handle_type();
        p_config["brake_local_handle_type"] = get_cntrl_local_brake_handle_type();
        p_config["brake_valve_type"] = get_valve_type();
        // LocHandle is unconditionally non-null after mover init (Mover.cpp's own switch always
        // assigns a TDriverHandle default), so "!= nullptr" never actually distinguishes "has a
        // real local handle" from "has none" - cntrl_local_brake_handle_type is the real signal.
        p_config["brake_local_handle_available"] = get_cntrl_local_brake_handle_type() != BRAKE_HANDLE_TYPE_NO_HANDLE;
        p_config["brake_max_cylinder_pressure"] = get_max_cylinder_pressure();
        p_config["brake_max_control_pressure"] =
                get_max_aux_pressure() >= 0.01 ? get_max_aux_pressure() : get_max_cylinder_pressure();

        if (p_mover->Handle == nullptr) {
            return;
        }
        p_config["brakes_controller_position_min"] = p_mover->Handle->GetPos(bh_MIN);
        p_config["brakes_controller_position_max"] = p_mover->Handle->GetPos(bh_MAX);
        p_config["brakes_controller_position_cutoff"] = p_mover->Handle->GetPos(bh_NP);
        p_config["brakes_controller_position_emergency"] = p_mover->Handle->GetPos(bh_EB);
    }

    void TrainBrake::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        const double brake_controller_pos = p_mover->fBrakeCtrlPos;
        const double brake_controller_min = p_mover->Handle->GetPos(bh_MIN);
        const double brake_controller_max = p_mover->Handle->GetPos(bh_MAX);
        double brake_controller_pos_normalized = 0.0;
        if (brake_controller_max != brake_controller_min) {
            brake_controller_pos_normalized =
                    (brake_controller_pos - brake_controller_min) / (brake_controller_max - brake_controller_min);
        }
        // Train.cpp:1839-1872 (OnCommand_alarmchaintoggle) - "alarmchain:"/ggAlarmChain
        // (Train.cpp:10027), manual emergency brake pull cord.
        p_state["alarm_chain_pulled"] = p_mover->AlarmChainFlag;
        p_state["brake_air_pressure"] = p_mover->BrakePress;
        p_state["brake_loco_pressure"] = p_mover->LocBrakePress;
        p_state["brake_pipe_pressure"] = p_mover->PipeBrakePress;
        p_state["pipe_pressure"] = p_mover->PipePress;
        p_state["brake_tank_volume"] = p_mover->Volume;
        p_state["brake_controller_position"] = brake_controller_pos;
        p_state["brake_controller_position_normalized"] = brake_controller_pos_normalized;
        // LocalBrakePosA ("nastawa hamulca pomocniczego") is already normalized 0..1 in the
        // mover, unlike the main brake's arbitrary Handle-position units above.
        p_state["brake_local_position_normalized"] = p_mover->LocalBrakePosA;

        p_state["brake_unit_force"] = p_mover->UnitBrakeForce;
        const double brake_force_max_per_block =
                p_mover->BrakeForceR(1.0, p_mover->Vel) / (std::max(1, p_mover->NAxles) * std::max(1, p_mover->NBpA));
        p_state["brake_force_ratio"] =
                std::clamp(p_mover->UnitBrakeForce / std::max(1.0, brake_force_max_per_block), 0.0, 1.0);
        p_state["brake_emergency_valve_flow"] = p_mover->EmergencyValveFlow;
        const double main_valve_flow = p_mover->dpMainValve;
        p_state["brake_main_valve_flow"] = std::isfinite(main_valve_flow) ? main_valve_flow : 0.0;
        p_state["brake_local_valve_flow"] = p_mover->dpLocalValve;
        // Original engine: Train.cpp's m_localbrakepressurechange (10x the low-pass-filtered
        // rate of LocBrakePress change, factor 0.1/frame) - see this member's own doc comment in
        // TrainBrake.hpp for why the sound layer needs this instead of the raw dpLocalValve
        // field above. Published as two already-non-negative magnitudes, matching how
        // brake_sfx_event_factory.gd's local-brake-hiss automation consumes them (one track per
        // sign, same shape as the original's separate rsSBHiss/rsSBHissU sounds).
        {
            const double dt = get_process_delta_time();
            if (local_brake_pressure_previous >= 0.0 && dt > 0.0) {
                const double raw_rate = 10.0 * ((p_mover->LocBrakePress - local_brake_pressure_previous) / dt);
                local_brake_pressure_change_rate = local_brake_pressure_change_rate * 0.9 + raw_rate * 0.1;
            }
            local_brake_pressure_previous = p_mover->LocBrakePress;
        }
        p_state["brake_loco_pressure_fall_rate"] = std::max(0.0, -local_brake_pressure_change_rate);
        p_state["brake_loco_pressure_rise_rate"] = std::max(0.0, local_brake_pressure_change_rate);
        p_state["brake_control_pressure"] = p_mover->LocHandle ? p_mover->LocHandle->GetCP() : 0.0;
        p_state["brake_local_aeim_position"] = p_mover->LocalBrakePosAEIM;
        p_state["brake_edb_cylinder_pressure"] = p_mover->Hamulec ? p_mover->Hamulec->GetEDBCP() : 0.0;
        p_state["brake_releaser_active"] = p_mover->Hamulec && p_mover->Hamulec->Releaser();
    }

    void TrainBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        /* logika z Mover::LoadFiz_Brake */
        p_mover->BrakeSystem = brake_system_type_map.at(cntrl_brake_system);               // BrakeSystem
        p_mover->BrakeCtrlPosNo = cntrl_brake_ctrl_position_count;                         // BCPN
        p_mover->BrakeDelay[0] = cntrl_brake_delay_1;                                      // BDelay1
        p_mover->BrakeDelay[1] = cntrl_brake_delay_2;                                      // BDelay2
        p_mover->BrakeDelay[2] = cntrl_brake_delay_3;                                      // BDelay3
        p_mover->BrakeDelay[3] = cntrl_brake_delay_4;                                      // BDelay4
        p_mover->BrakeDelays = cntrl_brake_delays;                                         // BrakeDelays
        p_mover->BrakeOpModes = cntrl_brake_op_modes;                                      // BrakeOpModes
        p_mover->BrakeHandle = brake_handle_type_map.at(cntrl_brake_handle_type);          // BrakeHandle
        p_mover->BrakeLocHandle = brake_handle_type_map.at(cntrl_local_brake_handle_type); // LocBrakeHandle
        p_mover->ASBType = cntrl_anti_skid_brake_type;                                     // ASB
        p_mover->LocalBrake = local_brake_type_map.at(cntrl_local_brake_type);             // LocalBrake
        p_mover->MBrake = cntrl_manual_brake_present;                                      // ManualBrake
        p_mover->LocHandleTimeTraxx = cntrl_local_brake_traxx;                             // LocalBrakeTraxx
        p_mover->DynamicBrakeType = cntrl_dynamic_brake_type;                              // DynamicBrake
        p_mover->ReleaseParkingBySpringBrake = cntrl_release_parking_by_spring_brake;
        p_mover->ReleaseParkingBySpringBrakeWhenDoorIsOpen = cntrl_release_parking_by_spring_brake_when_door_open;
        p_mover->SpringBrakeCutsOffDrive = cntrl_spring_brake_cuts_off_drive;
        p_mover->SpringBrakeDriveEmergencyVel = cntrl_spring_brake_drive_emergency_velocity;

        /* FIXME: BrakeValve nie jest tylko enumem, jesli w FIZ wpisze sie nieznany symbol zawierający ESt, to EXE
         * ustawi BrakeValve=ESt3. Powinien to ogarnąć importer FIZ
         *
         * Whoever thought making BrakeValve half-enum, half-parser-voodoo was a good idea
         * condemned everyone else to cargo-cult their bugs. Thanks a lot, dear original MaSzyna code authors.
         */

        // assuming same int values between our TrainBrakeValve and mover's TBrakeValve
        p_mover->BrakeValve = static_cast<TBrakeValve>(static_cast<int>(valve_type));

        const std::unordered_map<TBrakeValve, TBrakeSubSystem>::const_iterator it =
                brake_valve_to_subsystem_map.find(p_mover->BrakeValve);
        p_mover->BrakeSubsystem = it != brake_valve_to_subsystem_map.end() ? it->second : TBrakeSubSystem::ss_None;

        p_mover->NBpA = CLAMP<int, int, int>(friction_elements_per_axle, 0, 4);
        p_mover->MaxBrakeForce = brake_force_max;
        p_mover->BrakeValveSize = est_valve_size;
        p_mover->TrackBrakeForce = brake_force_traction * 1000.0;
        p_mover->MaxBrakePress[3] = max_cylinder_pressure;
        if (max_cylinder_pressure > 0.0) {
            p_mover->BrakeCylNo = cylinder_count;

            if (cylinder_count > 0) {
                p_mover->MaxBrakePress[0] = max_aux_pressure < 0.01 ? max_cylinder_pressure : max_aux_pressure;
                p_mover->MaxBrakePress[1] = max_tare_pressure;
                p_mover->MaxBrakePress[2] = max_medium_pressure;
                p_mover->MaxBrakePress[4] = max_antislip_pressure < 0.01 ? 0.0 : max_antislip_pressure;

                p_mover->BrakeCylRadius = cylinder_radius;
                p_mover->BrakeCylDist = cylinder_distance;
                p_mover->BrakeCylSpring = cylinder_spring_force;
                p_mover->BrakeSlckAdj = piston_stroke_adjuster_resistance;
                p_mover->BrakeRigEff = rig_effectiveness;

                p_mover->BrakeCylMult[0] = cylinder_gear_ratio;
                p_mover->BrakeCylMult[1] = cylinder_gear_ratio_low;
                p_mover->BrakeCylMult[2] = cylinder_gear_ratio_high;

                p_mover->P2FTrans = 100 * M_PI * std::pow(cylinder_radius, 2);

                p_mover->LoadFlag = (cylinder_gear_ratio_low > 0.0 || max_tare_pressure > 0.0) ? 1 : 0;

                p_mover->BrakeVolume = M_PI * std::pow(cylinder_radius, 2) * cylinder_distance * cylinder_count;
                p_mover->BrakeVVolume = tank_volume_aux;

                const std::unordered_map<BrakeMethod, int>::const_iterator lookup;
                p_mover->BrakeMethod = lookup != brake_method_map.find(brake_method) ? brake_method : 0;
                p_mover->BrakeMethod = brake_method;
                p_mover->RapidMult = rapid_transfer;
                p_mover->RapidVel = rapid_switching_speed;
            }
        } else {
            p_mover->P2FTrans = 0;
        }

        p_mover->CntrlPipePress =
                5 + (0.001 * (UtilityFunctions::randf_range(0.0, 10.0) - UtilityFunctions::randf_range(0.0, 10.0)));
        /* PipePress i HighPipePress musza byc skopiowane */
        p_mover->HighPipePress = pipe_pressure_max;
        p_mover->LowPipePress = pipe_pressure_min;
        p_mover->VeselVolume = tank_volume_main;
        p_mover->MinCompressor = compressor_cab_a_min_pressure;
        p_mover->MaxCompressor = compressor_cab_a_max_pressure;
        p_mover->MinCompressor_cabB = compressor_cab_b_min_pressure;
        p_mover->MaxCompressor_cabB = compressor_cab_b_max_pressure;

        p_mover->CompressorTankValve = compressor_tank_valve_active;
        p_mover->EmergencyValveOff = compressor_lower_emergency_closing_pressure;
        p_mover->EmergencyValveOn = compressor_higher_emergency_closing_pressure;

        p_mover->EmergencyValveArea = compressor_emergency_valve_area;
        p_mover->UniversalBrakeButtonFlag[0] = universal_brake_button_1;
        p_mover->UniversalBrakeButtonFlag[1] = universal_brake_button_2;
        p_mover->UniversalBrakeButtonFlag[2] = universal_brake_button_3;

        p_mover->LockPipeOn = main_pipe_blocking_pressure;
        p_mover->LockPipeOff = main_pipe_unblocking_pressure;
        p_mover->HandleUnlock = main_pipe_minimum_unblocking_handle_position;
        p_mover->EmergencyCutsOffHandle = false; //@TODO: Figure out wtf is this

        p_mover->CompressorSpeed = compressor_speed;
        p_mover->CompressorPower = compressor_power;

        // According to the original code - the parameter is provided in the form of a multiplier, where 1.0 means the
        // default rate of 0.01
        p_mover->AirLeakRate = air_leak_multiplier * 0.01;

        // By default, this should be set to true if an engine type is diesel or diesel-electric and false, otherwise
        //  this action should be performed by FIZ parser
        p_mover->ReleaserEnabledOnlyAtNoPowerPos = releaser_enabled_only_at_no_power_pos;
        if (p_mover->MinCompressor_cabB > 0.0) {
            p_mover->MinCompressor_cabA = p_mover->MinCompressor;
            p_mover->CabDependentCompressor = true;
        } else {
            p_mover->MinCompressor_cabB = p_mover->MinCompressor;
        }
        if (p_mover->MaxCompressor_cabB > 0.0) {
            p_mover->MaxCompressor_cabA = p_mover->MaxCompressor;
            p_mover->CabDependentCompressor = true;
        } else {
            p_mover->MaxCompressor_cabB = p_mover->MaxCompressor;
        }

        /* BPT: tabelka hamulcowa, wyszczegolnienie cisnien w rurze wg pozycji krana */
        p_mover->BrakePressureTable.clear();
        for (int i = 0; i < brake_pressure_table.size(); i++) {
            const Ref<BrakePressureTableItem> &row = brake_pressure_table[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[TrainBrake]: brake_pressure_table property is null at index " + String::num(i));
                continue;
            }
            Maszyna::TBrakePressure entry;
            entry.PipePressureVal = row->get_pipe_pressure();
            entry.BrakePressureVal = row->get_brake_cylinder_pressure();
            entry.FlowSpeedVal = row->get_fill_speed();
            entry.BrakeType = brake_pressure_table_type_map.at(row->get_brake_type());
            p_mover->BrakePressureTable[row->get_handle_position()] = entry;
        }

        /* CompressorList: programator sprezarek */
        constexpr int MAX_COMPRESSOR_LIST = 8;
        const int compressor_list_size = static_cast<int>(compressor_list.size());
        if (compressor_list_size > MAX_COMPRESSOR_LIST) {
            UtilityFunctions::push_warning(
                    "[TrainBrake]: compressor_list has " + String::num(compressor_list_size) +
                    " entries, exceeding the mover's limit of " + String::num(MAX_COMPRESSOR_LIST) + "; truncating.");
        }
        for (int i = 0; i < std::min(MAX_COMPRESSOR_LIST, compressor_list_size); i++) {
            const Ref<CompressorListItem> &row = compressor_list[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[TrainBrake]: compressor_list property is null at index " + String::num(i));
                continue;
            }
            p_mover->CompressorList[Maszyna::TCompressorList::cl_Allow][i + 1] = row->get_allow();
            p_mover->CompressorList[Maszyna::TCompressorList::cl_SpeedFactor][i + 1] = row->get_speed_factor();
            p_mover->CompressorList[Maszyna::TCompressorList::cl_MinFactor][i + 1] = row->get_min_pressure_factor();
            p_mover->CompressorList[Maszyna::TCompressorList::cl_MaxFactor][i + 1] = row->get_max_pressure_factor();
        }
    }
} // namespace godot
