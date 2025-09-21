#include "../core/utils.hpp"
#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainBrake::_bind_methods() {
        BIND_PROPERTY_W_HINT(Variant::INT, "valve_type", "valve/type", &TrainBrake::set_valve_type, &TrainBrake::get_valve_type, "valve_type", PROPERTY_HINT_ENUM, "NoValve,W,W_Lu_VI,W_Lu_L,W_Lu_XR,K,Kg,Kp,Kss,Kkg,Kkp,Kks,Hikg1,Hikss,Hikp1,KE,SW,EStED,NESt3,ESt3,LSt,ESt4,ESt3AL2,EP1,EP2,M483,CV1_L_TR,CV1,CV1_R,Other")
        BIND_PROPERTY(Variant::INT, "friction_elements_per_axle", "friction_elements_per_axle", &TrainBrake::set_friction_elements_per_axle, &TrainBrake::get_friction_elements_per_axle, "friction_elements_per_axle");
        BIND_PROPERTY(Variant::FLOAT, "max_brake_force", "brake_force/max", &TrainBrake::set_max_brake_force, &TrainBrake::get_max_brake_force, "max_brake_force");
        BIND_PROPERTY(Variant::INT, "est_valve_size", "est_valve/size", &TrainBrake::set_valve_size, &TrainBrake::get_valve_size, "valve_size");
        BIND_PROPERTY(Variant::FLOAT, "traction_brake_force", "brake_force/traction", &TrainBrake::set_traction_brake_force, &TrainBrake::get_traction_brake_force, "traction_brake_force");
        BIND_PROPERTY(Variant::FLOAT, "max_cylinder_pressure", "max_cylinder_pressure", &TrainBrake::set_max_cylinder_pressure, &TrainBrake::get_max_cylinder_pressure, "max_cylinder_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_aux_pressure", "max_aux_pressure", &TrainBrake::set_max_aux_pressure, &TrainBrake::get_max_aux_pressure, "max_aux_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_tare_pressure", "max_tare_pressure", &TrainBrake::set_max_tare_pressure, &TrainBrake::get_max_tare_pressure, "max_tare_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_medium_pressure", "max_medium_pressure", &TrainBrake::set_max_medium_pressure, &TrainBrake::get_max_medium_pressure, "max_medium_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_antislip_pressure", "max_antislip_pressure", &TrainBrake::set_max_antislip_pressure, &TrainBrake::get_max_antislip_pressure, "max_antislip_pressure");
        BIND_PROPERTY(Variant::INT, "cylinder_count", "cylinder/count", &TrainBrake::set_cylinder_count, &TrainBrake::get_cylinder_count, "cylinder_count");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_radius", "cylinder/radius", &TrainBrake::set_cylinder_radius, &TrainBrake::get_cylinder_radius, "cylinder_radius");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_distance", "cylinder/distance", &TrainBrake::set_cylinder_distance, &TrainBrake::get_cylinder_distance, "cylinder_distance");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_spring_force", "cylinder/spring_force", &TrainBrake::set_cylinder_spring_force, &TrainBrake::get_cylinder_spring_force, "cylinder_spring_force");
        BIND_PROPERTY(Variant::FLOAT, "piston_stroke_adjuster_resistance", "piston_stroke/adjuster_resistance", &TrainBrake::set_piston_stroke_adjuster_resistance, &TrainBrake::get_piston_stroke_adjuster_resistance, "");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_gear_ratio", "cylinder/gear_ratio", &TrainBrake::set_cylinder_gear_ratio, &TrainBrake::get_cylinder_gear_ratio, "cylinder_gear_ratio");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_gear_ratio_low", "cylinder/gear_ratio_low", &TrainBrake::set_cylinder_gear_ratio_low, &TrainBrake::get_cylinder_gear_ratio_low, "cylinder_gear_ratio_low");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_gear_ratio_high", "cylinder/gear_ratio_high", &TrainBrake::set_cylinder_gear_ratio_high, &TrainBrake::get_cylinder_gear_ratio_high, "cylinder_gear_ratio_high");
        BIND_PROPERTY(Variant::FLOAT, "pipe_pressure_min", "pipe/pressure_min", &TrainBrake::set_pipe_pressure_min, &TrainBrake::get_pipe_pressure_min, "pipe_pressure_min");
        BIND_PROPERTY(Variant::FLOAT, "pipe_pressure_max", "pipe/pressure_max", &TrainBrake::set_pipe_pressure_max, &TrainBrake::get_pipe_pressure_max, "pipe_pressure_max");
        BIND_PROPERTY(Variant::FLOAT, "main_tank_volume", "tank/volume_main", &TrainBrake::set_main_tank_volume, &TrainBrake::get_main_tank_volume, "main_tank_volume");
        BIND_PROPERTY(Variant::FLOAT, "aux_tank_volume", "tank/volume_aux", &TrainBrake::set_aux_tank_volume, &TrainBrake::get_aux_tank_volume, "aux_tank_volume");
        BIND_PROPERTY(Variant::FLOAT, "compressor_pressure_cab_a_min", "compressor/cab_a/min_pressure", &TrainBrake::set_compressor_pressure_cab_a_min, &TrainBrake::get_compressor_pressure_cab_a_min, "compressor_pressure_min");
        BIND_PROPERTY(Variant::FLOAT, "compressor_pressure_cab_a_max", "compressor/cab_a/max_pressure", &TrainBrake::set_compressor_pressure_cab_a_max, &TrainBrake::get_compressor_pressure_cab_a_max, "compressor_pressure_max");
        BIND_PROPERTY(Variant::FLOAT, "compressor_pressure_cab_b_min", "compressor/cab_b/min_pressure", &TrainBrake::set_compressor_pressure_cab_b_min, &TrainBrake::get_compressor_pressure_cab_b_min, "compressor_pressure_min");
        BIND_PROPERTY(Variant::FLOAT, "compressor_pressure_cab_b_max", "compressor/cab_b/max_pressure", &TrainBrake::set_compressor_pressure_cab_b_max, &TrainBrake::get_compressor_pressure_cab_b_max, "compressor_pressure_max");
        BIND_PROPERTY(Variant::FLOAT, "compressor_speed", "compressor/speed", &TrainBrake::set_compressor_speed, &TrainBrake::get_compressor_speed, "compressor_speed");
        BIND_PROPERTY_W_HINT(Variant::INT, "compressor_power", "compressor/power", &TrainBrake::set_compressor_power, &TrainBrake::get_compressor_power, "compressor_power", PROPERTY_HINT_ENUM, "Main,Unused,Converter,Engine,Coupler1,Coupler2");
        BIND_PROPERTY(Variant::FLOAT, "rig_effectiveness", "rig_effectiveness", &TrainBrake::set_rig_effectiveness, &TrainBrake::get_rig_effectiveness, "rig_effectiveness");
        BIND_PROPERTY_W_HINT(Variant::INT, "brake_method", "brake/method", &TrainBrake::set_brake_method, &TrainBrake::get_brake_method, "brake_method", PROPERTY_HINT_ENUM, "P10-Bg,P10-Bgu,FR513,FR510,Cosid,P10yBg,P10yBgu,Disk1,Disk1+Mg,Disk2");
        BIND_PROPERTY(Variant::FLOAT, "rapid_transfer", "rapid/transfer", &TrainBrake::set_rapid_transfer, &TrainBrake::get_rapid_transfer, "rapid_transfer");
        BIND_PROPERTY(Variant::FLOAT, "rapid_switching_speed", "rapid/switching_speed", &TrainBrake::set_rapid_switching_speed, &TrainBrake::get_rapid_switching_speed, "rapid_switching_speed");
        BIND_PROPERTY(Variant::FLOAT, "air_leak_multiplier", "air_leak_multiplier", &TrainBrake::set_air_leak_multiplier, &TrainBrake::get_air_leak_multiplier, "air_leak_multiplier")
        BIND_PROPERTY(Variant::BOOL, "compressor_tank_valve_active", "compressor/tank_valve_active", &TrainBrake::set_compressor_tank_valve_active, &TrainBrake::get_compressor_tank_valve_active, "compressor_tank_valve_active")
        BIND_PROPERTY(Variant::FLOAT, "lower_emergency_closing_pressure", "compressor/lower_emergency_closing_pressure", &TrainBrake::set_lower_emergency_closing_pressure, &TrainBrake::get_lower_emergency_closing_pressure, "lower_emergency_closing_pressure")
        BIND_PROPERTY(Variant::FLOAT, "higher_emergency_closing_pressure", "compressor/higher_emergency_closing_pressure", &TrainBrake::set_higher_emergency_closing_pressure, &TrainBrake::get_higher_emergency_closing_pressure, "higher_emergency_closing_pressure")
        BIND_PROPERTY(Variant::FLOAT, "main_pipe_blocking_pressure", "main_pipe/blocking_pressure", &TrainBrake::set_main_pipe_blocking_pressure, &TrainBrake::get_main_pipe_blocking_pressure, "main_pipe_blocking_pressure")
        BIND_PROPERTY(Variant::FLOAT, "main_pipe_unblocking_pressure", "main_pipe/unblocking_pressure", &TrainBrake::set_main_pipe_unblocking_pressure, &TrainBrake::get_main_pipe_unblocking_pressure, "main_pipe_unblocking_pressure")
        BIND_PROPERTY(Variant::FLOAT, "main_pipe_minimum_unblocking_handle_position", "main_pipe/minimum_unblocking_handle_position", &TrainBrake::set_main_pipe_minimum_unblocking_handle_position, &TrainBrake::get_main_pipe_minimum_unblocking_handle_position, "main_pipe_minimum_unblocking_handle_position")

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

        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10Bgu);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10Bg);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_D1);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_D2);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_FR513);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_Cosid);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10yBg);
        BIND_ENUM_CONSTANT(BRAKE_METHOD_P10yBgu);
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
        const double brake_controller_pos = brake_controller_min + (level * (brake_controller_max - brake_controller_min));
        mover->BrakeLevelSet(brake_controller_pos);
    }

    void TrainBrake::brake_level_set_position(const BrakeHandlePosition p_position) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        if (const std::unordered_map<BrakeHandlePosition, int>::const_iterator it = BrakeHandlePositionMap.find(
                p_position); it != BrakeHandlePositionMap.end()) {
            mover->BrakeLevelSet(mover->Handle->GetPos(it->second));
        } else {
            log_error("Unhandled brake level position: " + String::num(static_cast<int>(p_position)));
        }
    }

    void TrainBrake::brake_level_set_position_str(const String &p_position) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER_BRAKE(mover);
        const std::unordered_map<std::string, int>::const_iterator it =
                BrakeHandlePositionStringMap.find(std::string(p_position.utf8()));
        if (it != BrakeHandlePositionStringMap.end()) {
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

    void TrainBrake::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        if (mover->Handle == nullptr) {
            return;
        }
        config["brakes_controller_position_min"] = mover->Handle->GetPos(bh_MIN);
        config["brakes_controller_position_max"] = mover->Handle->GetPos(bh_MAX);
    }

    void TrainBrake::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        const double brake_controller_pos = mover->fBrakeCtrlPos;
        const double brake_controller_min = mover->Handle->GetPos(bh_MIN);
        const double brake_controller_max = mover->Handle->GetPos(bh_MAX);
        double brake_controller_pos_normalized = 0.0;
        if (brake_controller_max != brake_controller_min) {
            brake_controller_pos_normalized =
                    (brake_controller_pos - brake_controller_min) / (brake_controller_max - brake_controller_min);
        }
        state["brake_air_pressure"] = mover->BrakePress;
        state["brake_loco_pressure"] = mover->LocBrakePress;
        state["brake_pipe_pressure"] = mover->PipeBrakePress;
        state["pipe_pressure"] = mover->PipePress;
        state["brake_tank_volume"] = mover->Volume;
        state["brake_controller_position"] = brake_controller_pos;
        state["brake_controller_position_normalized"] = brake_controller_pos_normalized;
    }

    void TrainBrake::_do_update_internal_mover(TMoverParameters *mover) {
        /* logika z Mover::LoadFiz_Brake */
        mover->BrakeSystem = TBrakeSystem::Pneumatic;    // BrakeSystem
        mover->BrakeCtrlPosNo = 6;                       // BCPN
        mover->BrakeDelay[0] = 15;                       // BDelay1
        mover->BrakeDelay[1] = 3;                        // BDelay2
        mover->BrakeDelay[2] = 36;                       // BDelay3
        mover->BrakeDelay[3] = 22;                       // BDelay4
        mover->BrakeDelays = bdelay_G + bdelay_P;        // BrakeDelays
        mover->BrakeHandle = TBrakeHandle::FV4a;         // BrakeHandle
        mover->BrakeLocHandle = TBrakeHandle::FD1;       // LocBrakeHandle
        mover->ASBType = 1;                              // ASB
        mover->LocalBrake = TLocalBrake::PneumaticBrake; // LocalBrake
        mover->MBrake = true;                            // ManualBrake

        /* FIXME: BrakeValve nie jest tylko enumem, jesli w FIZ wpisze sie nieznany symbol zawierający ESt, to EXE ustawi BrakeValve=ESt3. Powinien to ogarnąć importer FIZ
         *
         * Whoever thought making BrakeValve half-enum, half-parser-voodoo was a good idea
         * condemned everyone else to cargo-cult their bugs. Thanks a lot, dear original MaSzyna code authors.
         */

        // assuming same int values between our TrainBrakeValve and mover's TBrakeValve
        mover->BrakeValve = static_cast<TBrakeValve>(static_cast<int>(valve_type));

        const std::unordered_map<TBrakeValve, TBrakeSubSystem>::const_iterator it =
                BrakeValveToSubsystemMap.find(mover->BrakeValve);
        mover->BrakeSubsystem = it != BrakeValveToSubsystemMap.end() ? it->second : TBrakeSubSystem::ss_None;

        mover->NBpA = CLAMP<int, int, int>(friction_elements_per_axle, 0, 4);
        mover->MaxBrakeForce = max_brake_force;
        mover->BrakeValveSize = valve_size;
        mover->TrackBrakeForce = traction_brake_force * 1000.0;
        mover->MaxBrakePress[3] = max_cylinder_pressure;
        if (max_cylinder_pressure > 0.0) {
            mover->BrakeCylNo = cylinder_count;

            if (cylinder_count > 0) {
                mover->MaxBrakePress[0] = max_aux_pressure < 0.01 ? max_cylinder_pressure : max_aux_pressure;
                mover->MaxBrakePress[1] = max_tare_pressure;
                mover->MaxBrakePress[2] = max_medium_pressure;
                mover->MaxBrakePress[4] = max_antislip_pressure < 0.01 ? 0.0 : max_antislip_pressure;

                mover->BrakeCylRadius = cylinder_radius;
                mover->BrakeCylDist = cylinder_distance;
                mover->BrakeCylSpring = cylinder_spring_force;
                mover->BrakeSlckAdj = piston_stroke_adjuster_resistance;
                mover->BrakeRigEff = rig_effectiveness;

                mover->BrakeCylMult[0] = cylinder_gear_ratio;
                mover->BrakeCylMult[1] = cylinder_gear_ratio_low;
                mover->BrakeCylMult[2] = cylinder_gear_ratio_high;

                mover->P2FTrans = 100 * M_PI * std::pow(cylinder_radius, 2);

                mover->LoadFlag = (cylinder_gear_ratio_low > 0.0 || max_tare_pressure > 0.0) ? 1 : 0;

                mover->BrakeVolume = M_PI * std::pow(cylinder_radius, 2) * cylinder_distance * cylinder_count;
                mover->BrakeVVolume = aux_tank_volume;

                const std::unordered_map<BrakeMethod, int>::const_iterator lookup;
                mover->BrakeMethod = lookup != BrakeMethodMap.find(brake_method) ? brake_method : 0;
                mover->BrakeMethod = brake_method;
                mover->RapidMult = rapid_transfer;
                mover->RapidVel = rapid_switching_speed;
            }
        } else {
            mover->P2FTrans = 0;
        }

        mover->CntrlPipePress = 5 + 0.001 * (libmaszyna::utils::random(0, 10) - libmaszyna::utils::random(0, 10));
        /* PipePress i HighPipePress musza byc skopiowane */
        mover->HighPipePress = pipe_pressure_max;
        mover->LowPipePress = pipe_pressure_min;
        mover->VeselVolume = main_tank_volume;
        mover->MinCompressor = compressor_pressure_cab_a_min;
        mover->MaxCompressor = compressor_pressure_cab_a_max;
        mover->MinCompressor_cabB = compressor_pressure_cab_b_min;
        mover->MaxCompressor_cabB = compressor_pressure_cab_b_max;

        mover->CompressorTankValve = compressor_tank_valve_active;
        mover->EmergencyValveOff = lower_emergency_closing_pressure;
        mover->EmergencyValveOn = higher_emergency_closing_pressure;

        //@TODO: Figure out and implement equivalents for UniversalBrakeButtonFlag

        mover->LockPipeOn = main_pipe_blocking_pressure;
        mover->LockPipeOff = main_pipe_unblocking_pressure;
        mover->HandleUnlock = main_pipe_minimum_unblocking_handle_position;
        mover->EmergencyCutsOffHandle = false; //@TODO: Figure out wtf is this

        mover->CompressorSpeed = compressor_speed;
        mover->CompressorPower = compressor_power;

        //According to the original code - the parameter is provided in the form of a multiplier, where 1.0 means the default rate of 0.01
        mover->AirLeakRate = air_leak_multiplier*0.01;

        //By default, this should be set to true if an engine type is diesel or diesel-electric and false, otherwise
        // this action should be performed by FIZ parser
        mover->ReleaserEnabledOnlyAtNoPowerPos = releaser_enabled_only_at_no_power_pos;
        if (mover->MinCompressor_cabB > 0.0) {
            mover->MinCompressor_cabA = mover->MinCompressor;
            mover->CabDependentCompressor = true;
        }
        else {
            mover->MinCompressor_cabB = mover->MinCompressor;
        }
        if (mover->MaxCompressor_cabB > 0.0)
        {
            mover->MaxCompressor_cabA = mover->MaxCompressor;
            mover->CabDependentCompressor = true;
        }
        else {
            mover->MaxCompressor_cabB = mover->MaxCompressor;
        }
    }
} // namespace godot
