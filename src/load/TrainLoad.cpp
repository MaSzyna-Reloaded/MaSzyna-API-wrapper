#include "TrainLoad.hpp"

namespace godot {
    void TrainLoad::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "load_list", "load_list", &TrainLoad::set_load_list, &TrainLoad::get_load_list, "load_list", PROPERTY_HINT_TYPE_STRING, "LoadListItem")
        BIND_PROPERTY_W_HINT(Variant::INT, "load_unit", "load_unit", &TrainLoad::set_load_unit, &TrainLoad::get_load_unit, "load_unit", PROPERTY_HINT_ENUM, "Tons,Pieces");
        BIND_PROPERTY(Variant::FLOAT, "overload_factor", "overload_factor", &TrainLoad::set_overload_factor, &TrainLoad::get_overload_factor, "overload_factor");
        BIND_PROPERTY(Variant::FLOAT, "load_speed", "load_speed", &TrainLoad::set_load_speed, &TrainLoad::get_load_speed, "load_speed");
        BIND_PROPERTY(Variant::FLOAT, "unload_speed", "unload_speed", &TrainLoad::set_unload_speed, &TrainLoad::get_unload_speed, "unload_speed");

        BIND_ENUM_CONSTANT(LOAD_UNIT_TONS);
        BIND_ENUM_CONSTANT(LOAD_UNIT_PIECES);
    }

    void TrainLoad::_do_update_internal_mover(TMoverParameters *mover) {
        //mover->MaxLoad -> This is loaded by original LoadFIZ_Load logic, but the variable doesn't exist in the mover's code at all
        //mover->LoadAccepted -> This is loaded by original LoadFIZ_Load logic, but the variable doesn't exist in the mover's code at all
        mover->LoadQuantity = load_unit == LOAD_UNIT_TONS ? "tons" : "pieces";
        mover->LoadSpeed = load_speed;
        mover->UnLoadSpeed = unload_speed;
        mover->OverLoadFactor = overload_factor;
        TrainPart::_do_update_internal_mover(mover);
    }

    void TrainLoad::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {}

    void TrainLoad::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        TrainPart::_do_fetch_config_from_mover(mover, config);
    }

    void TrainLoad::_register_commands() {
        TrainPart::_register_commands();
    }

    void TrainLoad::_unregister_commands() {
        TrainPart::_unregister_commands();
    }
} // namespace godot