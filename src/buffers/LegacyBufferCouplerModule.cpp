#include "../core/GameLog.hpp"
#include "../core/LegacyRailVehicle.hpp"
#include "LegacyBufferCouplerModule.hpp"

namespace godot {
    static void _export_coupler_state(Dictionary &state, const String &prefix, const TCoupling &coupler) {
        state[prefix + String("connected")] = coupler.Connected != nullptr;
        state[prefix + String("connected_number")] = coupler.ConnectedNr;
    }

    void LegacyBufferCouplerModule::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                Variant::INT, "coupler_type", "coupler/type", &LegacyBufferCouplerModule::set_coupler_type,
                &LegacyBufferCouplerModule::get_coupler_type, "value", PROPERTY_HINT_ENUM,
                "Automatic,Screw,Chain,Bare,Articulated");
        BIND_PROPERTY(
                Variant::FLOAT, "buffer_stiffness_k", "buffer/stiffness_k",
                &LegacyBufferCouplerModule::set_buffer_stiffness_k, &LegacyBufferCouplerModule::get_buffer_stiffness_k,
                "value");
        BIND_PROPERTY(
                Variant::FLOAT, "buffer_max_compression_tolerance", "buffer/max_compression_tolerance",
                &LegacyBufferCouplerModule::set_buffer_max_compression_tolerance,
                &LegacyBufferCouplerModule::get_buffer_max_compression_tolerance, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "buffer_max_tension_tolerance", "buffer/max_tension_tolerance",
                &LegacyBufferCouplerModule::set_buffer_max_tension_tolerance,
                &LegacyBufferCouplerModule::get_buffer_max_tension_tolerance, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "coupler_stiffness_k", "coupler/stiffness_k",
                &LegacyBufferCouplerModule::set_coupler_stiffness_k,
                &LegacyBufferCouplerModule::get_coupler_stiffness_k, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "coupler_max_compression_tolerance", "coupler/max_compression_tolerance",
                &LegacyBufferCouplerModule::set_coupler_max_compression_tolerance,
                &LegacyBufferCouplerModule::get_coupler_max_compression_tolerance, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "coupler_max_tension_tolerance", "coupler/max_tension_tolerance",
                &LegacyBufferCouplerModule::set_coupler_max_tension_tolerance,
                &LegacyBufferCouplerModule::get_coupler_max_tension_tolerance, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "damping_beta", "damping_beta", &LegacyBufferCouplerModule::set_damping_beta,
                &LegacyBufferCouplerModule::get_damping_beta, "value");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "allowed_flag", "allowed_flag", &LegacyBufferCouplerModule::set_allowed_flag,
                &LegacyBufferCouplerModule::get_allowed_flag, "value", PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V "
                "electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "automatic_flag", "automatic_flag", &LegacyBufferCouplerModule::set_automatic_flag,
                &LegacyBufferCouplerModule::get_automatic_flag, "value", PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V "
                "electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "power_flag", "power_flag", &LegacyBufferCouplerModule::set_power_flag,
                &LegacyBufferCouplerModule::get_power_flag, "value", PROPERTY_HINT_FLAGS, "24V,110V,3x400V");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "power_coupling", "power_coupling", &LegacyBufferCouplerModule::set_power_coupling,
                &LegacyBufferCouplerModule::get_power_coupling, "value", PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V "
                "electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY(
                Variant::STRING, "control_type", "control_type", &LegacyBufferCouplerModule::set_control_type,
                &LegacyBufferCouplerModule::get_control_type, "value");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "buffer_location", "buffer_location", &LegacyBufferCouplerModule::set_buffer_location,
                &LegacyBufferCouplerModule::get_buffer_location, "value", PROPERTY_HINT_ENUM, "Front,Back,Both");

        BIND_ENUM_CONSTANT(COUPLER_TYPE_AUTOMATIC)
        BIND_ENUM_CONSTANT(COUPLER_TYPE_SCREW)
        BIND_ENUM_CONSTANT(COUPLER_TYPE_CHAIN)
        BIND_ENUM_CONSTANT(COUPLER_TYPE_BARE)
        BIND_ENUM_CONSTANT(COUPLER_TYPE_ARTICULATED)

        BIND_ENUM_CONSTANT(ALLOWED_MECHANICAL)
        BIND_ENUM_CONSTANT(ALLOWED_BRAKE_PIPE)
        BIND_ENUM_CONSTANT(ALLOWED_MULTIPLE_CONTROL)
        BIND_ENUM_CONSTANT(ALLOWED_HIGH_VOLTAGE)
        BIND_ENUM_CONSTANT(ALLOWED_PASSAGE)
        BIND_ENUM_CONSTANT(ALLOWED_AIR_8_BAR)
        BIND_ENUM_CONSTANT(ALLOWED_HEATING)
        BIND_ENUM_CONSTANT(ALLOWED_FIXED_COUPLING_LOCK)
        BIND_ENUM_CONSTANT(ALLOWED_ELEC_24V)
        BIND_ENUM_CONSTANT(ALLOWED_ELEC_110V)
        BIND_ENUM_CONSTANT(ALLOWED_ELEC_3x400V)

        BIND_ENUM_CONSTANT(POWER_24V)
        BIND_ENUM_CONSTANT(POWER_110V)
        BIND_ENUM_CONSTANT(POWER_3x400V)

        BIND_ENUM_CONSTANT(BUFFER_LOCATION_FRONT)
        BIND_ENUM_CONSTANT(BUFFER_LOCATION_BACK)
        BIND_ENUM_CONSTANT(BUFFER_LOCATION_BOTH)
    }

    void LegacyBufferCouplerModule::_do_update_internal_mover(TMoverParameters *mover) {
        if (mover == nullptr) {
            return;
        }
        LegacyRailVehicle *vehicle = get_legacy_rail_vehicle_node();
        if (vehicle == nullptr) {
            return;
        }

        TCoupling *coupler = nullptr;
        if (buffer_location == BufferLocation::BUFFER_LOCATION_FRONT) {
            coupler = &mover->Couplers[0];
        } else {
            coupler = &mover->Couplers[1];
        }

        const double mass = vehicle->get_mass();
        const double max_velocity = vehicle->get_max_velocity();
        std::map<CouplerType, TCouplerType> coupler_types{
                {COUPLER_TYPE_AUTOMATIC, TCouplerType::Automatic},
                {COUPLER_TYPE_SCREW, TCouplerType::Screw},
                {COUPLER_TYPE_CHAIN, TCouplerType::Chain},
                {COUPLER_TYPE_BARE, TCouplerType::Bare},
                {COUPLER_TYPE_ARTICULATED, TCouplerType::Articulated},
        };

        const auto lookup = coupler_types.find(coupler_type);
        const TCouplerType resolved_type = lookup != coupler_types.end() ? lookup->second : TCouplerType::NoCoupler;

        coupler->CouplerType = resolved_type;
        coupler->SpringKC = coupler_stiffness_k;
        coupler->DmaxC = coupler_max_compression_tolerance;
        coupler->FmaxC = coupler_max_tension_tolerance;
        coupler->SpringKB = buffer_stiffness_k;
        coupler->DmaxB = buffer_max_compression_tolerance;
        coupler->FmaxB = buffer_max_tension_tolerance;
        coupler->beta = damping_beta;
        coupler->AutomaticCouplingFlag = automatic_flag;
        coupler->AllowedFlag = allowed_flag;
        if (coupler->AllowedFlag < 0) {
            coupler->AllowedFlag = -coupler->AllowedFlag | coupling::permanent;
        }

        coupler->PowerCoupling = power_coupling;
        coupler->PowerFlag = power_flag;
        coupler->control_type = control_type.ascii();

        if (coupler->CouplerType != TCouplerType::NoCoupler && coupler->CouplerType != TCouplerType::Bare &&
            coupler->CouplerType != TCouplerType::Articulated) {
            coupler->SpringKC *= 1000;
            coupler->FmaxC *= 1000;
            coupler->SpringKB *= 1000;
            coupler->FmaxB *= 1000;
        } else if (coupler->CouplerType == TCouplerType::Bare) {
            coupler->SpringKC = (50.0 * mass) + (max_velocity / 0.05);
            coupler->DmaxC = 0.05;
            coupler->FmaxC = (100.0 * mass) + (2 * max_velocity);
            coupler->SpringKB = (60.0 * mass) + (max_velocity / 0.05);
            coupler->DmaxB = 0.05;
            coupler->FmaxB = (50.0 * mass) + (2.0 * max_velocity);
            coupler->beta = 0.3;
        } else if (coupler->CouplerType == TCouplerType::Articulated) {
            coupler->SpringKC = 4500 * 1000;
            coupler->DmaxC = 0.05;
            coupler->FmaxC = 850 * 1000;
            coupler->SpringKB = 9200 * 1000;
            coupler->DmaxB = 0.05;
            coupler->FmaxB = 320 * 1000;
            coupler->beta = 0.55;
        }

        if (buffer_location == BufferLocation::BUFFER_LOCATION_BOTH) {
            mover->Couplers[0] = mover->Couplers[1];
        }

        const String line = String("[LegacyBufferCouplerModule] apply ") + vehicle->get_name() +
                            " location=" + String::num_int64(static_cast<int>(buffer_location)) +
                            " front_type=" + String::num_int64(static_cast<int>(mover->Couplers[end::front].type())) +
                            " front_kc=" + String::num(mover->Couplers[end::front].SpringKC) +
                            " front_dmaxc=" + String::num(mover->Couplers[end::front].DmaxC) +
                            " front_fmaxc=" + String::num(mover->Couplers[end::front].FmaxC) +
                            " rear_type=" + String::num_int64(static_cast<int>(mover->Couplers[end::rear].type())) +
                            " rear_kc=" + String::num(mover->Couplers[end::rear].SpringKC) +
                            " rear_dmaxc=" + String::num(mover->Couplers[end::rear].DmaxC) +
                            " rear_fmaxc=" + String::num(mover->Couplers[end::rear].FmaxC);
        DEBUG("%s", line);
    }

    void LegacyBufferCouplerModule::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        if (mover == nullptr) {
            return;
        }

        if (buffer_location == BufferLocation::BUFFER_LOCATION_FRONT ||
            buffer_location == BufferLocation::BUFFER_LOCATION_BOTH) {
            _export_coupler_state(state, "coupler_front_", mover->Couplers[end::front]);
        }

        if (buffer_location == BufferLocation::BUFFER_LOCATION_BACK ||
            buffer_location == BufferLocation::BUFFER_LOCATION_BOTH) {
            _export_coupler_state(state, "coupler_back_", mover->Couplers[end::rear]);
        }
    }

    void LegacyBufferCouplerModule::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        config.set("buffer_stiffness_k", buffer_stiffness_k);
        config.set("buffer_max_compression_tolerance", buffer_max_compression_tolerance);
        config.set("buffer_max_tension_tolerance", buffer_max_tension_tolerance);
        config.set("coupler_stiffness_k", coupler_stiffness_k);
        config.set("coupler_max_compression_tolerance", coupler_max_compression_tolerance);
        config.set("coupler_max_tension_tolerance", coupler_max_tension_tolerance);
        config.set("damping_beta", damping_beta);
        config.set("allowed_flag", allowed_flag);
        config.set("automatic_flag", automatic_flag);
        config.set("power_flag", power_flag);
        config.set("power_coupling", power_coupling);
        config.set("control_type", control_type);
        config.set("coupler_type", static_cast<int>(coupler_type));
        config.set("buffer_location", static_cast<int>(buffer_location));
    }
} // namespace godot
