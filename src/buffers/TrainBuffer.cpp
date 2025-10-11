#include "TrainBuffer.hpp"

namespace godot {
    void TrainBuffer::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                Variant::INT, "coupler_type", "coupler/type", &TrainBuffer::set_coupler_type,
                &TrainBuffer::get_coupler_type, "value", PROPERTY_HINT_ENUM,
                "Automatic,Screw,Chain,Bare,Articulated");

        // Buffer properties
        BIND_PROPERTY(
                Variant::FLOAT, "buffer_stiffness_k", "buffer/stiffness_k", &TrainBuffer::set_buffer_stiffness_k,
                &TrainBuffer::get_buffer_stiffness_k, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "buffer_max_compression_tolerance", "buffer/max_compression_tolerance",
                &TrainBuffer::set_buffer_max_compression_tolerance, &TrainBuffer::get_buffer_max_compression_tolerance,
                "value");
        BIND_PROPERTY(
                Variant::FLOAT, "buffer_max_tension_tolerance", "buffer/max_tension_tolerance",
                &TrainBuffer::set_buffer_max_tension_tolerance, &TrainBuffer::get_buffer_max_tension_tolerance,
                "value");

        // Coupler properties
        BIND_PROPERTY(
                Variant::FLOAT, "coupler_stiffness_k", "coupler/stiffness_k", &TrainBuffer::set_coupler_stiffness_k,
                &TrainBuffer::get_coupler_stiffness_k, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "coupler_max_compression_tolerance", "coupler/max_compression_tolerance",
                &TrainBuffer::set_coupler_max_compression_tolerance,
                &TrainBuffer::get_coupler_max_compression_tolerance, "value");
        BIND_PROPERTY(
                Variant::FLOAT, "coupler_max_tension_tolerance", "coupler/max_tension_tolerance",
                &TrainBuffer::set_coupler_max_tension_tolerance, &TrainBuffer::get_coupler_max_tension_tolerance,
                "value");

        // Damping
        BIND_PROPERTY(
                Variant::FLOAT, "damping_beta", "damping_beta", &TrainBuffer::set_damping_beta,
                &TrainBuffer::get_damping_beta, "value");

        // Coupler capability flags and control
        BIND_PROPERTY_W_HINT(
                Variant::INT, "allowed_flag", "allowed_flag", &TrainBuffer::set_allowed_flag,
                &TrainBuffer::get_allowed_flag, "value", PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "automatic_flag", "automatic_flag", &TrainBuffer::set_automatic_flag,
                &TrainBuffer::get_automatic_flag, "value", PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "power_flag", "power_flag", &TrainBuffer::set_power_flag, &TrainBuffer::get_power_flag,
                "value", PROPERTY_HINT_FLAGS, "24V,110V,3x400V");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "power_coupling", "power_coupling", &TrainBuffer::set_power_coupling,
                &TrainBuffer::get_power_coupling, "value", PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY(
                Variant::STRING, "control_type", "control_type", &TrainBuffer::set_control_type,
                &TrainBuffer::get_control_type, "value");

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
    }

    void TrainBuffer::_do_update_internal_mover(TMoverParameters *mover) {
        ASSERT_MOVER(mover)
        TCoupling *coupler;
        if (buffer_location == BufferLocation::BUFFER_LOCATION_FRONT) {
            coupler = &mover->Couplers[0];
        } else {
            coupler = &mover->Couplers[1];
        }
        const double mass = train_controller_node->get_mass();
        const double max_velocity = train_controller_node->get_max_velocity();
        std::map<CouplerType, TCouplerType> _coupler_types{
                {COUPLER_TYPE_AUTOMATIC, TCouplerType::Automatic},
                {COUPLER_TYPE_SCREW, TCouplerType::Screw},
                {COUPLER_TYPE_CHAIN, TCouplerType::Chain},
                {COUPLER_TYPE_BARE, TCouplerType::Bare},
                {COUPLER_TYPE_ARTICULATED, TCouplerType::Articulated},
        };

        const std::map<CouplerType, TCouplerType>::iterator lookup = _coupler_types.find(coupler_type);
        const TCouplerType resolved_type = lookup != _coupler_types.end() ? lookup->second : TCouplerType::NoCoupler;

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

        if (coupler->CouplerType != TCouplerType::NoCoupler
            && coupler->CouplerType != TCouplerType::Bare
            && coupler->CouplerType != TCouplerType::Articulated) {

            coupler->SpringKC *= 1000;
            coupler->FmaxC *= 1000;
            coupler->SpringKB *= 1000;
            coupler->FmaxB *= 1000;
        } else if (coupler->CouplerType == TCouplerType::Bare) {

            coupler->SpringKC = 50.0 * mass + max_velocity / 0.05;
            coupler->DmaxC = 0.05;
            coupler->FmaxC = 100.0 * mass + 2 * max_velocity;
            coupler->SpringKB = 60.0 * mass + max_velocity / 0.05;
            coupler->DmaxB = 0.05;
            coupler->FmaxB = 50.0 * mass + 2.0 * max_velocity;
            coupler->beta = 0.3;
        } else if (coupler->CouplerType == TCouplerType::Articulated) {
            /*
                    coupler.SpringKC = 60.0 * Mass + 1000;
                    coupler.DmaxC = 0.05;
                    coupler.FmaxC = 20000000.0 + 2.0 * Ftmax;
                    coupler.SpringKB = 70.0 * Mass + 1000;
                    coupler.DmaxB = 0.05;
                    coupler.FmaxB = 4000000.0 + 2.0 * Ftmax;
                    coupler.beta = 0.55;
            */
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
    }

    void TrainBuffer::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        // Expose current config/state if needed
    }

    void TrainBuffer::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        // Provide config dictionary entries
        config["buffer_stiffness_k"] = buffer_stiffness_k;
        config["buffer_max_compression_tolerance"] = buffer_max_compression_tolerance;
        config["buffer_max_tension_tolerance"] = buffer_max_tension_tolerance;
        config["coupler_stiffness_k"] = coupler_stiffness_k;
        config["coupler_max_compression_tolerance"] = coupler_max_compression_tolerance;
        config["coupler_max_tension_tolerance"] = coupler_max_tension_tolerance;
        config["damping_beta"] = damping_beta;

        // New flags and control properties
        config["allowed_flag"] = allowed_flag;
        config["automatic_flag"] = automatic_flag;
        config["power_flag"] = power_flag;
        config["power_coupling"] = power_coupling;
        config["control_type"] = control_type;
    }
} //namespace godot

