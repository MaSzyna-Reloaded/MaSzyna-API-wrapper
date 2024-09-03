#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "../brakes/TrainBrake.hpp"
#include "../core/TrainController.hpp"

namespace godot {

    TrainBrake::TrainBrake() = default;

    void TrainBrake::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_brake_level"), &TrainBrake::set_brake_level);
        ClassDB::bind_method(D_METHOD("get_brake_level"), &TrainBrake::get_brake_level);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "brake_level"), "set_brake_level", "get_brake_level");

        ClassDB::bind_method(D_METHOD("set_valve"), &TrainBrake::set_valve);
        ClassDB::bind_method(D_METHOD("get_valve"), &TrainBrake::get_valve);

        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "valve/type", PROPERTY_HINT_ENUM,
                        "NoValve,W,W_Lu_VI,W_Lu_L,W_Lu_XR,K,Kg,Kp,Kss,Kkg,Kkp,Kks,Hikg1,Hikss,Hikp1,KE,SW,"
                        "EStED,NESt3,ESt3,LSt,ESt4,ESt3AL2,EP1,EP2,M483,CV1_L_TR,CV1,CV1_R,Other"),
                "set_valve", "get_valve");
        ClassDB::bind_method(
                D_METHOD("set_friction_elements_per_axle", "friction_elements_per_axle"),
                &TrainBrake::set_friction_elements_per_axle);
        ClassDB::bind_method(D_METHOD("get_friction_elements_per_axle"), &TrainBrake::get_friction_elements_per_axle);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "friction_elements_per_axle"), "set_friction_elements_per_axle",
                "get_friction_elements_per_axle");

        ClassDB::bind_method(D_METHOD("set_max_brake_force"), &TrainBrake::set_max_brake_force);
        ClassDB::bind_method(D_METHOD("get_max_brake_force"), &TrainBrake::get_max_brake_force);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "brake_force/max"), "set_max_brake_force", "get_max_brake_force");

        ClassDB::bind_method(D_METHOD("set_valve_size"), &TrainBrake::set_valve_size);
        ClassDB::bind_method(D_METHOD("get_valve_size"), &TrainBrake::get_valve_size);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "valve/est_size"), "set_valve_size", "get_valve_size");

        ClassDB::bind_method(D_METHOD("set_track_brake_force"), &TrainBrake::set_track_brake_force);
        ClassDB::bind_method(D_METHOD("get_track_brake_force"), &TrainBrake::get_track_brake_force);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "brake_force/track"), "set_track_brake_force", "get_track_brake_force");
        ClassDB::bind_method(D_METHOD("set_max_pressure"), &TrainBrake::set_max_pressure);
        ClassDB::bind_method(D_METHOD("get_max_pressure"), &TrainBrake::get_max_pressure);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_pressures/cylinder"), "set_max_pressure", "get_max_pressure");

        ClassDB::bind_method(D_METHOD("set_max_pressure_aux"), &TrainBrake::set_max_pressure_aux);
        ClassDB::bind_method(D_METHOD("get_max_pressure_aux"), &TrainBrake::get_max_pressure_aux);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_pressures/aux"), "set_max_pressure_aux", "get_max_pressure_aux");

        ClassDB::bind_method(D_METHOD("set_max_pressure_tare"), &TrainBrake::set_max_pressure_tare);
        ClassDB::bind_method(D_METHOD("get_max_pressure_tare"), &TrainBrake::get_max_pressure_tare);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "max_pressures/tare"), "set_max_pressure_tare", "get_max_pressure_tare");

        ClassDB::bind_method(D_METHOD("set_max_pressure_medium"), &TrainBrake::set_max_pressure_medium);
        ClassDB::bind_method(D_METHOD("get_max_pressure_medium"), &TrainBrake::get_max_pressure_medium);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "max_pressures/medium"), "set_max_pressure_medium",
                "get_max_pressure_medium");

        ClassDB::bind_method(D_METHOD("set_max_antislip_pressure"), &TrainBrake::set_max_antislip_pressure);
        ClassDB::bind_method(D_METHOD("get_max_antislip_pressure"), &TrainBrake::get_max_antislip_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "max_pressures/antislip"), "set_max_antislip_pressure",
                "get_max_antislip_pressure");

        ClassDB::bind_method(D_METHOD("set_cylinders_count"), &TrainBrake::set_cylinders_count);
        ClassDB::bind_method(D_METHOD("get_cylinders_count"), &TrainBrake::get_cylinders_count);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "cylinders/count"), "set_cylinders_count", "get_cylinders_count");

        ClassDB::bind_method(D_METHOD("set_cylinder_radius"), &TrainBrake::set_cylinder_radius);
        ClassDB::bind_method(D_METHOD("get_cylinder_radius"), &TrainBrake::get_cylinder_radius);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cylinders/radius"), "set_cylinder_radius", "get_cylinder_radius");

        ClassDB::bind_method(D_METHOD("set_cylinder_distance"), &TrainBrake::set_cylinder_distance);
        ClassDB::bind_method(D_METHOD("get_cylinder_distance"), &TrainBrake::get_cylinder_distance);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "cylinders/distance"), "set_cylinder_distance", "get_cylinder_distance");

        ClassDB::bind_method(D_METHOD("set_cylinder_spring_force"), &TrainBrake::set_cylinder_spring_force);
        ClassDB::bind_method(D_METHOD("get_cylinder_spring_force"), &TrainBrake::get_cylinder_spring_force);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "cylinders/spring_force"), "set_cylinder_spring_force",
                "get_cylinder_spring_force");

        ClassDB::bind_method(D_METHOD("set_slck_adjustment_force"), &TrainBrake::set_slck_adjustment_force);
        ClassDB::bind_method(D_METHOD("get_slck_adjustment_force"), &TrainBrake::get_slck_adjustment_force);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "slck/adjustment_force"), "set_slck_adjustment_force",
                "get_slck_adjustment_force");

        ClassDB::bind_method(D_METHOD("set_cylinder_gear_ratio"), &TrainBrake::set_cylinder_gear_ratio);
        ClassDB::bind_method(D_METHOD("get_cylinder_gear_ratio"), &TrainBrake::get_cylinder_gear_ratio);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "cylinders/gear_ratio"), "set_cylinder_gear_ratio",
                "get_cylinder_gear_ratio");

        ClassDB::bind_method(D_METHOD("set_cylinder_gear_ratio_low"), &TrainBrake::set_cylinder_gear_ratio_low);
        ClassDB::bind_method(D_METHOD("get_cylinder_gear_ratio_low"), &TrainBrake::get_cylinder_gear_ratio_low);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "cylinders/gear_ratio_low"), "set_cylinder_gear_ratio_low",
                "get_cylinder_gear_ratio_low");

        ClassDB::bind_method(D_METHOD("set_cylinder_gear_ratio_high"), &TrainBrake::set_cylinder_gear_ratio_high);
        ClassDB::bind_method(D_METHOD("get_cylinder_gear_ratio_high"), &TrainBrake::get_cylinder_gear_ratio_high);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "cylinders/gear_ratio_high"), "set_cylinder_gear_ratio_high",
                "get_cylinder_gear_ratio_high");

        ClassDB::bind_method(D_METHOD("set_pipe_pressure_min"), &TrainBrake::set_pipe_pressure_min);
        ClassDB::bind_method(D_METHOD("get_pipe_pressure_min"), &TrainBrake::get_pipe_pressure_min);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "pipe/pressure_min"), "set_pipe_pressure_min", "get_pipe_pressure_min");

        ClassDB::bind_method(D_METHOD("set_pipe_pressure_max"), &TrainBrake::set_pipe_pressure_max);
        ClassDB::bind_method(D_METHOD("get_pipe_pressure_max"), &TrainBrake::get_pipe_pressure_max);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "pipe/pressure_max"), "set_pipe_pressure_max", "get_pipe_pressure_max");

        ClassDB::bind_method(D_METHOD("set_main_tank_volume"), &TrainBrake::set_main_tank_volume);
        ClassDB::bind_method(D_METHOD("get_main_tank_volume"), &TrainBrake::get_main_tank_volume);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tank/volume_main"), "set_main_tank_volume", "get_main_tank_volume");

        ClassDB::bind_method(D_METHOD("set_aux_tank_volume"), &TrainBrake::set_aux_tank_volume);
        ClassDB::bind_method(D_METHOD("get_aux_tank_volume"), &TrainBrake::get_aux_tank_volume);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tank/volume_aux"), "set_aux_tank_volume", "get_aux_tank_volume");

        ClassDB::bind_method(D_METHOD("set_compressor_pressure_min"), &TrainBrake::set_compressor_pressure_min);
        ClassDB::bind_method(D_METHOD("get_compressor_pressure_min"), &TrainBrake::get_compressor_pressure_min);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "compressor/pressure_min"), "set_compressor_pressure_min",
                "get_compressor_pressure_min");

        ClassDB::bind_method(D_METHOD("set_compressor_pressure_max"), &TrainBrake::set_compressor_pressure_max);
        ClassDB::bind_method(D_METHOD("get_compressor_pressure_max"), &TrainBrake::get_compressor_pressure_max);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "compressor/pressure_max"), "set_compressor_pressure_max",
                "get_compressor_pressure_max");

        ClassDB::bind_method(
                D_METHOD("set_compressor_pressure_cab_b_min"), &TrainBrake::set_compressor_pressure_cab_b_min);
        ClassDB::bind_method(
                D_METHOD("get_compressor_pressure_cab_b_min"), &TrainBrake::get_compressor_pressure_cab_b_min);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "compressor/pressure_cab_b_min"), "set_compressor_pressure_cab_b_min",
                "get_compressor_pressure_cab_b_min");

        ClassDB::bind_method(
                D_METHOD("set_compressor_pressure_cab_b_max"), &TrainBrake::set_compressor_pressure_cab_b_max);
        ClassDB::bind_method(
                D_METHOD("get_compressor_pressure_cab_b_max"), &TrainBrake::get_compressor_pressure_cab_b_max);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "compressor/pressure_cab_b_max"), "set_compressor_pressure_cab_b_max",
                "get_compressor_pressure_cab_b_max");

        ClassDB::bind_method(D_METHOD("set_compressor_speed"), &TrainBrake::set_compressor_speed);
        ClassDB::bind_method(D_METHOD("get_compressor_speed"), &TrainBrake::get_compressor_speed);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "compressor/speed"), "set_compressor_speed", "get_compressor_speed");

        ClassDB::bind_method(D_METHOD("set_compressor_power"), &TrainBrake::set_compressor_power);
        ClassDB::bind_method(D_METHOD("get_compressor_power"), &TrainBrake::get_compressor_power);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "compressor/power", PROPERTY_HINT_ENUM,
                        "Main,UNUSED,Converter,Engine,Coupler1,Coupler2"),
                "set_compressor_power", "get_compressor_power");

        ClassDB::bind_method(D_METHOD("set_rig_effectiveness"), &TrainBrake::set_rig_effectiveness);
        ClassDB::bind_method(D_METHOD("get_rig_effectiveness"), &TrainBrake::get_rig_effectiveness);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rig_effectiveness"), "set_rig_effectiveness", "get_rig_effectiveness");

        ClassDB::bind_method(D_METHOD("set_sw_releaser_enabled"), &TrainBrake::set_sw_releaser_enabled);
        ClassDB::bind_method(D_METHOD("get_sw_releaser_enabled"), &TrainBrake::get_sw_releaser_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/releaser"), "set_sw_releaser_enabled", "get_sw_releaser_enabled");
    }

    void TrainBrake::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["brake_air_pressure"] = mover->BrakePress;
        state["brake_loco_pressure"] = mover->LocBrakePress;
        state["brake_pipe_pressure"] = mover->PipeBrakePress;
        state["pipe_pressure"] = mover->PipePress;
        state["brake_tank_volume"] = mover->Volume;
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
         */
        mover->BrakeValve = static_cast<TBrakeValve>(valve);

        auto it = BrakeValveToSubsystemMap.find(mover->BrakeValve);
        mover->BrakeSubsystem = it != BrakeValveToSubsystemMap.end() ? it->second : TBrakeSubSystem::ss_None;

        mover->NBpA = friction_elements_per_axle;
        mover->MaxBrakeForce = max_brake_force;
        mover->BrakeValveSize = valve_size;
        mover->TrackBrakeForce = track_brake_force * 1000.0;
        mover->MaxBrakePress[3] = max_pressure;
        if (max_pressure > 0.0) {
            mover->BrakeCylNo = cylinders_count;

            if (cylinders_count > 0) {
                mover->MaxBrakePress[0] = max_pressure_aux < 0.01 ? max_pressure : max_pressure_aux;
                mover->MaxBrakePress[1] = max_pressure_tare;
                mover->MaxBrakePress[2] = max_pressure_medium;
                mover->MaxBrakePress[4] = max_antislip_pressure < 0.01 ? 0.0 : max_antislip_pressure;

                mover->BrakeCylRadius = cylinder_radius;
                mover->BrakeCylDist = cylinder_distance;
                mover->BrakeCylSpring = cylinder_spring_force;
                mover->BrakeSlckAdj = slck_adjustment_force;
                mover->BrakeRigEff = rig_effectiveness;

                mover->BrakeCylMult[0] = cylinder_gear_ratio;
                mover->BrakeCylMult[1] = cylinder_gear_ratio_low;
                mover->BrakeCylMult[2] = cylinder_gear_ratio_high;

                mover->P2FTrans = 100 * M_PI * std::pow(cylinder_radius, 2);

                mover->LoadFlag = (cylinder_gear_ratio_low > 0.0 || max_pressure_tare > 0.0) ? 1 : 0;

                mover->BrakeVolume = M_PI * std::pow(cylinder_radius, 2) * cylinder_distance * cylinders_count;
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
        mover->MinCompressor = compressor_pressure_min;
        mover->MaxCompressor = compressor_pressure_max;
        mover->MinCompressor_cabB = compressor_pressure_cab_b_min;
        mover->MaxCompressor_cabB = compressor_pressure_cab_b_max;
        mover->CompressorSpeed = compressor_speed;
        mover->CompressorPower = compressor_power;

        /* update the brake level state */
        if (mover->Hamulec != nullptr) {
            if (mover->fBrakeCtrlPos != brake_level) {
                mover->BrakeLevelSet(brake_level);
            }
            if (sw_releaser_enabled) {
                mover->BrakeReleaser(1);
            }
        }

        // FIXME: Hardcoded brake system params (move to TrainController??? not sure)
    }

    void TrainBrake::set_valve(const int p_valve) {
        valve = p_valve;
        _dirty = true;
    }

    int TrainBrake::get_valve() const {
        return valve;
    }

    void TrainBrake::set_friction_elements_per_axle(const int p_friction_elements_per_axle) {
        friction_elements_per_axle = p_friction_elements_per_axle;
        _dirty = true;
    }

    int TrainBrake::get_friction_elements_per_axle() const {
        return friction_elements_per_axle;
    }

    void TrainBrake::set_max_brake_force(const double p_max_brake_force) {
        max_brake_force = p_max_brake_force;
        _dirty = true;
    }

    double TrainBrake::get_max_brake_force() const {
        return max_brake_force;
    }

    void TrainBrake::set_valve_size(const int p_valve_size) {
        valve_size = p_valve_size;
        _dirty = true;
    }

    int TrainBrake::get_valve_size() const {
        return valve_size;
    }

    void TrainBrake::set_track_brake_force(const double p_track_brake_force) {
        track_brake_force = p_track_brake_force;
        _dirty = true;
    }

    double TrainBrake::get_track_brake_force() const {
        return track_brake_force;
    }

    void TrainBrake::set_max_pressure(const double p_max_pressure) {
        max_pressure = p_max_pressure;
        _dirty = true;
    }

    double TrainBrake::get_max_pressure() const {
        return max_pressure;
    }

    void TrainBrake::set_max_pressure_aux(const double p_value) {
        max_pressure_aux = p_value;
        _dirty = true;
    }

    double TrainBrake::get_max_pressure_aux() const {
        return max_pressure_aux;
    }

    void TrainBrake::set_max_pressure_tare(const double p_value) {
        max_pressure_tare = p_value;
        _dirty = true;
    }

    double TrainBrake::get_max_pressure_tare() const {
        return max_pressure_tare;
    }

    void TrainBrake::set_max_pressure_medium(const double p_value) {
        max_pressure_medium = p_value;
        _dirty = true;
    }

    double TrainBrake::get_max_pressure_medium() const {
        return max_pressure_medium;
    }

    void TrainBrake::set_max_antislip_pressure(const double p_max_antislip_pressure) {
        max_antislip_pressure = p_max_antislip_pressure;
        _dirty = true;
    }

    double TrainBrake::get_max_antislip_pressure() const {
        return max_antislip_pressure;
    }

    void TrainBrake::set_cylinders_count(const int p_cylinders_count) {
        cylinders_count = p_cylinders_count;
        _dirty = true;
    }

    int TrainBrake::get_cylinders_count() const {
        return cylinders_count;
    }

    void TrainBrake::set_cylinder_radius(const double p_cylinder_radius) {
        cylinder_radius = p_cylinder_radius;
        _dirty = true;
    }

    double TrainBrake::get_cylinder_radius() const {
        return cylinder_radius;
    }

    void TrainBrake::set_cylinder_distance(const double p_cylinder_distance) {
        cylinder_distance = p_cylinder_distance;
        _dirty = true;
    }

    double TrainBrake::get_cylinder_distance() const {
        return cylinder_distance;
    }

    void TrainBrake::set_cylinder_spring_force(const double p_cylinder_spring_force) {
        cylinder_spring_force = p_cylinder_spring_force;
        _dirty = true;
    }

    double TrainBrake::get_cylinder_spring_force() const {
        return cylinder_spring_force;
    }

    void TrainBrake::set_slck_adjustment_force(const double p_slck_adjustment_force) {
        slck_adjustment_force = p_slck_adjustment_force;
        _dirty = true;
    }

    double TrainBrake::get_slck_adjustment_force() const {
        return slck_adjustment_force;
    }

    void TrainBrake::set_cylinder_gear_ratio(const double p_cylinder_gear_ratio) {
        cylinder_gear_ratio = p_cylinder_gear_ratio;
    }

    double TrainBrake::get_cylinder_gear_ratio() const {
        return cylinder_gear_ratio;
    }

    void TrainBrake::set_cylinder_gear_ratio_low(const double p_cylinder_gear_ratio_low) {
        cylinder_gear_ratio_low = p_cylinder_gear_ratio_low;
        _dirty = true;
    }

    double TrainBrake::get_cylinder_gear_ratio_low() const {
        return cylinder_gear_ratio_low;
    }

    void TrainBrake::set_cylinder_gear_ratio_high(const double p_cylinder_gear_ratio_high) {
        cylinder_gear_ratio_high = p_cylinder_gear_ratio_high;
        _dirty = true;
    }

    double TrainBrake::get_cylinder_gear_ratio_high() const {
        return cylinder_gear_ratio_high;
    }

    void TrainBrake::set_pipe_pressure_max(const double p_pipe_pressure_max) {
        pipe_pressure_max = p_pipe_pressure_max;
        _dirty = true;
    }

    double TrainBrake::get_pipe_pressure_max() const {
        return pipe_pressure_max;
    }

    void TrainBrake::set_pipe_pressure_min(const double p_pipe_pressure_min) {
        pipe_pressure_min = std::max(3.5, p_pipe_pressure_min); // MOVER LoadFiz_Brake
        _dirty = true;
    }

    double TrainBrake::get_pipe_pressure_min() const {
        return pipe_pressure_min;
    }
    void TrainBrake::set_main_tank_volume(const double p_main_tank_volume) {
        main_tank_volume = p_main_tank_volume;
        _dirty = true;
    }

    double TrainBrake::get_main_tank_volume() const {
        return main_tank_volume;
    }

    void TrainBrake::set_aux_tank_volume(const double p_aux_tank_volume) {
        aux_tank_volume = p_aux_tank_volume;
        _dirty = true;
    }

    double TrainBrake::get_aux_tank_volume() const {
        return aux_tank_volume;
    }

    void TrainBrake::set_compressor_pressure_min(const double p_compressor_pressure_min) {
        compressor_pressure_min = p_compressor_pressure_min;
        _dirty = true;
    }

    double TrainBrake::get_compressor_pressure_min() const {
        return compressor_pressure_min;
    }

    void TrainBrake::set_compressor_pressure_max(const double p_compressor_pressure_max) {
        compressor_pressure_max = p_compressor_pressure_max;
        _dirty = true;
    }

    double TrainBrake::get_compressor_pressure_max() const {
        return compressor_pressure_max;
    }

    void TrainBrake::set_compressor_pressure_cab_b_min(const double p_compressor_pressure_cab_b_min) {
        compressor_pressure_cab_b_min = p_compressor_pressure_cab_b_min;
        _dirty = true;
    }

    double TrainBrake::get_compressor_pressure_cab_b_min() const {
        return compressor_pressure_cab_b_min;
    }

    void TrainBrake::set_compressor_pressure_cab_b_max(const double p_compressor_pressure_cab_b_max) {
        compressor_pressure_cab_b_max = p_compressor_pressure_cab_b_max;
        _dirty = true;
    }

    double TrainBrake::get_compressor_pressure_cab_b_max() const {
        return compressor_pressure_cab_b_max;
    }

    void TrainBrake::set_compressor_speed(const double p_compressor_speed) {
        compressor_speed = p_compressor_speed;
        _dirty = true;
    }

    double TrainBrake::get_compressor_speed() const {
        return compressor_speed;
    }

    void TrainBrake::set_compressor_power(const int p_compressor_power) {
        compressor_power = p_compressor_power;
        _dirty = true;
    }

    int TrainBrake::get_compressor_power() const {
        return compressor_power;
    }

    void TrainBrake::set_rig_effectiveness(const double p_rig_effectiveness) {
        rig_effectiveness = p_rig_effectiveness;
        _dirty = true;
    }

    double TrainBrake::get_rig_effectiveness() const {
        return rig_effectiveness;
    }

    void TrainBrake::set_brake_level(const double p_value) {
        brake_level = p_value >= 0.0 ? p_value : 0.0;
        _dirty = true;
    }

    double TrainBrake::get_brake_level() {
        return brake_level;
    }

    void TrainBrake::set_sw_releaser_enabled(bool p_value) {
        sw_releaser_enabled = p_value;
        _dirty = true;
    }

    bool TrainBrake::get_sw_releaser_enabled() {
        return sw_releaser_enabled;
    }

} // namespace godot
