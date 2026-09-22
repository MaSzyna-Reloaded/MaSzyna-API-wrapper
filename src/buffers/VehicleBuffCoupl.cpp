#include "VehicleBuffCoupl.hpp"

namespace godot {
    void VehicleBuffCoupl::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                VehicleBuffCoupl, Variant::INT, coupler_type, "coupler", PROPERTY_HINT_ENUM,
                "Automatic,Screw,Chain,Bare,Articulated");

        // Buffer properties
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, buffer_stiffness_k, "buffer");
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, buffer_max_compression_tolerance, "buffer");
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, buffer_max_tension_tolerance, "buffer");

        // Coupler properties
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, coupler_stiffness_k, "coupler");
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, coupler_max_compression_tolerance, "coupler");
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, coupler_max_tension_tolerance, "coupler");

        // Damping
        BIND_PROPERTY(VehicleBuffCoupl, Variant::FLOAT, damping_beta);

        // Coupler capability flags and control
        BIND_PROPERTY_W_HINT(
                VehicleBuffCoupl, Variant::INT, allowed_flag, PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V "
                "electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY_W_HINT(
                VehicleBuffCoupl, Variant::INT, automatic_flag, PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V "
                "electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY_W_HINT(VehicleBuffCoupl, Variant::INT, power_flag, PROPERTY_HINT_FLAGS, "24V,110V,3x400V");
        BIND_PROPERTY_W_HINT(
                VehicleBuffCoupl, Variant::INT, power_coupling, PROPERTY_HINT_FLAGS,
                "Mechanical,Brake pipe,Multiple control,High voltage,Passage,Air 8 bar,Heating,Fixed coupling lock,24V "
                "electric cable,110V electric cable,3+400V electric cable");
        BIND_PROPERTY(VehicleBuffCoupl, Variant::STRING, control_type);
        BIND_PROPERTY_W_HINT(VehicleBuffCoupl, Variant::INT, buffer_location, PROPERTY_HINT_ENUM, "Front,Back,Both");
        ClassDB::bind_method(D_METHOD("couple"), &VehicleBuffCoupl::couple);
        ClassDB::bind_method(D_METHOD("decouple"), &VehicleBuffCoupl::decouple);

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
        BIND_ENUM_CONSTANT(ALLOWED_ELEC_3X400_V)

        BIND_ENUM_CONSTANT(POWER_24V)
        BIND_ENUM_CONSTANT(POWER_110V)
        BIND_ENUM_CONSTANT(POWER_3X400_V)

        BIND_ENUM_CONSTANT(BUFFER_LOCATION_FRONT)
        BIND_ENUM_CONSTANT(BUFFER_LOCATION_BACK)
        BIND_ENUM_CONSTANT(BUFFER_LOCATION_BOTH)
    }

    void VehicleBuffCoupl::_register_commands() {
        register_command("buffer_couple", Callable(this, "couple"));
        register_command("buffer_decouple", Callable(this, "decouple"));
        VehicleComponent::_register_commands();
    }

    void VehicleBuffCoupl::_unregister_commands() {
        unregister_command("buffer_couple", Callable(this, "couple"));
        unregister_command("buffer_decouple", Callable(this, "decouple"));
        VehicleComponent::_unregister_commands();
    }
} // namespace godot
