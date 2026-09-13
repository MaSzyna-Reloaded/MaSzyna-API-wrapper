#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include "../core/utils.hpp"
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
                "P10-Bg,P10-Bgu,FR513,FR510,Cosid,P10yBg,P10yBgu,Disk1,Disk1+Mg,Disk2");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, rapid_transfer, "rapid");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, rapid_switching_speed, "rapid");
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, air_leak_multiplier)
        BIND_PROPERTY(TrainBrake, Variant::BOOL, compressor_tank_valve_active, "compressor")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_lower_emergency_closing_pressure, "compressor")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, compressor_higher_emergency_closing_pressure, "compressor")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, main_pipe_blocking_pressure, "main_pipe")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, main_pipe_unblocking_pressure, "main_pipe")
        BIND_PROPERTY(TrainBrake, Variant::FLOAT, main_pipe_minimum_unblocking_handle_position, "main_pipe")

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
    }

    void TrainBrake::_register_commands() {
        register_command("brake_releaser", Callable(this, "brake_releaser"));
        register_command("brake_level_set", Callable(this, "brake_level_set"));
        register_command("brake_level_set_position", Callable(this, "brake_level_set_position_str"));
        register_command("brake_level_increase", Callable(this, "brake_level_increase"));
        register_command("brake_level_decrease", Callable(this, "brake_level_decrease"));
    }

    void TrainBrake::_unregister_commands() {
        unregister_command("brake_releaser", Callable(this, "brake_releaser"));
        unregister_command("brake_level_set", Callable(this, "brake_level_set"));
        unregister_command("brake_level_set_position", Callable(this, "brake_level_set_position_str"));
        unregister_command("brake_level_increase", Callable(this, "brake_level_increase"));
        unregister_command("brake_level_decrease", Callable(this, "brake_level_decrease"));
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

    void TrainBrake::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        if (p_mover->Handle == nullptr) {
            return;
        }
        p_config["brakes_controller_position_min"] = p_mover->Handle->GetPos(bh_MIN);
        p_config["brakes_controller_position_max"] = p_mover->Handle->GetPos(bh_MAX);
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
        p_state["brake_air_pressure"] = p_mover->BrakePress;
        p_state["brake_loco_pressure"] = p_mover->LocBrakePress;
        p_state["brake_pipe_pressure"] = p_mover->PipeBrakePress;
        p_state["pipe_pressure"] = p_mover->PipePress;
        p_state["brake_tank_volume"] = p_mover->Volume;
        p_state["brake_controller_position"] = brake_controller_pos;
        p_state["brake_controller_position_normalized"] = brake_controller_pos_normalized;
    }

    void TrainBrake::_do_update_internal_mover(TMoverParameters *p_mover) {
        /* logika z Mover::LoadFiz_Brake */
        p_mover->BrakeSystem = TBrakeSystem::Pneumatic;    // BrakeSystem
        p_mover->BrakeCtrlPosNo = 6;                       // BCPN
        p_mover->BrakeDelay[0] = 15;                       // BDelay1
        p_mover->BrakeDelay[1] = 3;                        // BDelay2
        p_mover->BrakeDelay[2] = 36;                       // BDelay3
        p_mover->BrakeDelay[3] = 22;                       // BDelay4
        p_mover->BrakeDelays = bdelay_G + bdelay_P;        // BrakeDelays
        p_mover->BrakeHandle = TBrakeHandle::FV4a;         // BrakeHandle
        p_mover->BrakeLocHandle = TBrakeHandle::FD1;       // LocBrakeHandle
        p_mover->ASBType = 1;                              // ASB
        p_mover->LocalBrake = TLocalBrake::PneumaticBrake; // LocalBrake
        p_mover->MBrake = true;                            // ManualBrake

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

        //@TODO: Figure out and implement equivalents for UniversalBrakeButtonFlag

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
    }
} // namespace godot
