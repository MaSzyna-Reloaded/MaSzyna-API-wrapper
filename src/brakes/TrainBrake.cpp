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
        BIND_PROPERTY(Variant::FLOAT, "max_cylinder_pressure", "max_cylinder_pressure", &TrainBrake::set_max_cyl_pressure, &TrainBrake::get_max_cyl_pressure, "max_cylinder_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_aux_pressure", "max_aux_pressure", &TrainBrake::set_max_aux_pressure, &TrainBrake::get_max_aux_pressure, "max_aux_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_tare_pressure", "max_tare_pressure", &TrainBrake::set_max_tare_pressure, &TrainBrake::get_max_tare_pressure, "max_tare_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_medium_pressure", "max_medium_pressure", &TrainBrake::set_max_medium_pressure, &TrainBrake::get_max_medium_pressure, "max_medium_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_antislip_pressure", "max_antislip_pressure", &TrainBrake::set_max_antislip_pressure, &TrainBrake::get_max_antislip_pressure, "max_antislip_pressure");
        BIND_PROPERTY(Variant::INT, "cylinder_count", "cylinder/count", &TrainBrake::set_cyl_count, &TrainBrake::get_cyl_count, "cylinder_count");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_radius", "cylinder/radius", &TrainBrake::set_cyl_radius, &TrainBrake::get_cyl_radius, "cylinder_radius");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_distance", "cylinder/distance", &TrainBrake::set_cyl_distance, &TrainBrake::get_cyl_distance, "cylinder_distance");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_spring_force", "cylinder/spring_force", &TrainBrake::set_cyl_spring_force, &TrainBrake::get_cyl_spring_force, "cylinder_spring_force");
        BIND_PROPERTY(Variant::FLOAT, "piston_stroke_adjuster_resistance", "piston_stroke/adjuster_resistance", &TrainBrake::set_piston_stroke_adjuster_resistance, &TrainBrake::get_piston_stroke_adjuster_resistance, "");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_gear_ratio", "cylinder/gear_ratio", &TrainBrake::set_cyl_gear_ratio, &TrainBrake::get_cyl_gear_ratio, "cylinder_gear_ratio");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_gear_ratio_low", "cylinder/gear_ratio_low", &TrainBrake::set_cyl_gear_ratio_low, &TrainBrake::get_cyl_gear_ratio_low, "cylinder_gear_ratio_low");
        BIND_PROPERTY(Variant::FLOAT, "cylinder_gear_ratio_high", "cylinder/gear_ratio_high", &TrainBrake::set_cyl_gear_ratio_high, &TrainBrake::get_cyl_gear_ratio_high, "cylinder_gear_ratio_high");
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
        const auto it = BrakeHandlePositionMap.find(p_position);
        if (it != BrakeHandlePositionMap.end()) {
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
        // FIXME: logika nie jest jeszcze w pelni przeniesiona z LoadFIZ_Brake

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

        /* FIXME:: BrakeValve nie jest tylko enumem
         * jesli w FIZ wpisze sie nieznany symbol zawierający ESt, to EXE ustawi BrakeValve=ESt3
         * byc moze to powinien ogarnąć importer FIZ
         */

        // assuming same int values between our TrainBrakeValve and mover's TBrakeValve
        mover->BrakeValve = static_cast<TBrakeValve>(static_cast<int>(valve_type));

        const std::unordered_map<TBrakeValve, TBrakeSubSystem>::const_iterator it =
                BrakeValveToSubsystemMap.find(mover->BrakeValve);
        mover->BrakeSubsystem = it != BrakeValveToSubsystemMap.end() ? it->second : TBrakeSubSystem::ss_None;

        mover->NBpA = friction_elements_per_axle;
        mover->MaxBrakeForce = max_brake_force;
        mover->BrakeValveSize = valve_size;
        mover->TrackBrakeForce = traction_brake_force * 1000.0;
        mover->MaxBrakePress[3] = max_cyl_pressure;
        if (max_cyl_pressure > 0.0) {
            mover->BrakeCylNo = cyl_count;

            if (cyl_count > 0) {
                mover->MaxBrakePress[0] = max_aux_pressure < 0.01 ? max_cyl_pressure : max_aux_pressure;
                mover->MaxBrakePress[1] = max_tare_pressure;
                mover->MaxBrakePress[2] = max_medium_pressure;
                mover->MaxBrakePress[4] = max_antislip_pressure < 0.01 ? 0.0 : max_antislip_pressure;

                mover->BrakeCylRadius = cyl_radius;
                mover->BrakeCylDist = cyl_distance;
                mover->BrakeCylSpring = cyl_spring_force;
                mover->BrakeSlckAdj = piston_stroke_adjuster_resistance;
                mover->BrakeRigEff = rig_effectiveness;

                mover->BrakeCylMult[0] = cyl_gear_ratio;
                mover->BrakeCylMult[1] = cyl_gear_ratio_low;
                mover->BrakeCylMult[2] = cyl_gear_ratio_high;

                mover->P2FTrans = 100 * M_PI * std::pow(cyl_radius, 2);

                mover->LoadFlag = (cyl_gear_ratio_low > 0.0 || max_tare_pressure > 0.0) ? 1 : 0;

                mover->BrakeVolume = M_PI * std::pow(cyl_radius, 2) * cyl_distance * cyl_count;
                mover->BrakeVVolume = aux_tank_volume;

                // TODO: mover->BrakeMethod
                mover->BrakeMethod = 0;

                // TODO: RM -> mover->RapidMult
                // TODO: RV -> mover->RapidVel
                mover->RapidMult = 1;
                mover->RapidVel = 55;
            }
        }
        /* PipePress i HighPipePress musza byc skopiowane */
        mover->HighPipePress = pipe_pressure_max;
        mover->LowPipePress = pipe_pressure_min;
        mover->VeselVolume = main_tank_volume;
        mover->MinCompressor = compressor_pressure_cab_a_min;
        mover->MaxCompressor = compressor_pressure_cab_a_max;
        mover->MinCompressor_cabB = compressor_pressure_cab_b_min;
        mover->MaxCompressor_cabB = compressor_pressure_cab_b_max;
        mover->CompressorSpeed = compressor_speed;
        mover->CompressorPower = compressor_power;
    }
} // namespace godot
