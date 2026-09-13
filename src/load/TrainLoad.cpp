#include "TrainLoad.hpp"

namespace godot {
    void TrainLoad::_bind_methods() {
        BIND_PROPERTY_W_HINT_RES_ARRAY(TrainLoad, Variant::ARRAY, load_list, PROPERTY_HINT_ARRAY_TYPE, "LoadListItem")
        BIND_PROPERTY_W_HINT(TrainLoad, Variant::INT, load_unit, PROPERTY_HINT_ENUM, "Tons,Pieces");
        BIND_PROPERTY(TrainLoad, Variant::FLOAT, overload_factor);
        BIND_PROPERTY(TrainLoad, Variant::FLOAT, load_speed);
        BIND_PROPERTY(TrainLoad, Variant::FLOAT, unload_speed);
        BIND_PROPERTY(TrainLoad, Variant::FLOAT, max_load);
        BIND_PROPERTY_ARRAY(TrainLoad, minimum_load_offsets);
        BIND_PROPERTY_ARRAY(TrainLoad, accepted_loads);
        BIND_ENUM_CONSTANT(LOAD_UNIT_TONS);
        BIND_ENUM_CONSTANT(LOAD_UNIT_PIECES);
    }

    void TrainLoad::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->MaxLoad = max_load;
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
            p_mover->LoadAttributes.emplace_back(std::string(load_str.utf8().get_data()), min_offset);
        }
        p_mover->LoadQuantity = load_unit == LOAD_UNIT_TONS ? "tons" : "pieces";
        p_mover->LoadSpeed = load_speed;
        p_mover->UnLoadSpeed = unload_speed;
        p_mover->OverLoadFactor = static_cast<float>(overload_factor);
        TrainPart::_do_update_internal_mover(p_mover);
    }

    void TrainLoad::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {}

    void TrainLoad::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        TrainPart::_do_fetch_config_from_mover(p_mover, p_config);
    }

    void TrainLoad::_register_commands() {
        TrainPart::_register_commands();
    }

    void TrainLoad::_unregister_commands() {
        TrainPart::_unregister_commands();
    }
} // namespace godot
