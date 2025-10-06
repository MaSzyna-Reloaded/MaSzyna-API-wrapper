#include "TrainLoad.hpp"

namespace godot {
    void TrainLoad::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "load_list", "load_list", &TrainLoad::set_load_list, &TrainLoad::get_load_list, "load_list", PROPERTY_HINT_ARRAY_TYPE, "LoadListItem")
        BIND_PROPERTY_W_HINT(Variant::INT, "load_unit", "load_unit", &TrainLoad::set_load_unit, &TrainLoad::get_load_unit, "load_unit", PROPERTY_HINT_ENUM, "Tons,Pieces");
        BIND_PROPERTY(Variant::FLOAT, "overload_factor", "overload_factor", &TrainLoad::set_overload_factor, &TrainLoad::get_overload_factor, "overload_factor");
        BIND_PROPERTY(Variant::FLOAT, "load_speed", "load_speed", &TrainLoad::set_load_speed, &TrainLoad::get_load_speed, "load_speed");
        BIND_PROPERTY(Variant::FLOAT, "unload_speed", "unload_speed", &TrainLoad::set_unload_speed, &TrainLoad::get_unload_speed, "unload_speed");
        BIND_PROPERTY(Variant::FLOAT, "max_load", "max_load", &TrainLoad::set_max_load, &TrainLoad::get_max_load, "max_load");
        BIND_PROPERTY_ARRAY("minimum_load_offsets", "minimum_load_offsets", &TrainLoad::set_minimum_load_offsets, &TrainLoad::get_minimum_load_offsets, "minimum_load_offsets");
        BIND_PROPERTY_ARRAY("accepted_loads", "accepted_loads", &TrainLoad::set_accepted_loads, &TrainLoad::get_accepted_loads, "accepted_loads");
        BIND_ENUM_CONSTANT(LOAD_UNIT_TONS);
        BIND_ENUM_CONSTANT(LOAD_UNIT_PIECES);
    }

    void TrainLoad::_do_update_internal_mover(TMoverParameters *mover) {
        mover->MaxLoad = max_load;
        // Build LoadAttributes from accepted_loads with optional per-load minimum offset
        const int loads_count = static_cast<int>(accepted_loads.size());
        const int offsets_count = static_cast<int>(minimum_load_offsets.size());
        for (int i = 0; i < loads_count; ++i) {
            String load_str = accepted_loads[i]; // TypedArray<String> element
            float min_offset = 0.f;
            if (i < offsets_count) {
                // TypedArray<float> element may be represented as real Variant (double)
                min_offset = static_cast<float>(static_cast<double>(minimum_load_offsets[i]));
            }
            mover->LoadAttributes.emplace_back(std::string(load_str.utf8().get_data()), min_offset);
        }
        mover->LoadQuantity = load_unit == LOAD_UNIT_TONS ? "tons" : "pieces";
        mover->LoadSpeed = load_speed;
        mover->UnLoadSpeed = unload_speed;
        mover->OverLoadFactor = static_cast<float>(overload_factor);
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