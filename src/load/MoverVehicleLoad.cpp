#include "MoverVehicleLoad.hpp"
#include "VehicleLoad.hpp"

namespace godot {
    void MoverVehicleLoad::_bind_methods() {}


    void MoverVehicleLoad::_do_update_internal_mover(TMoverParameters *p_mover) {
        p_mover->MaxLoad = get_max_load();
        // Build LoadAttributes from get_accepted_loads() with optional per-load minimum offset
        const int loads_count = static_cast<int>(get_accepted_loads().size());
        const int offsets_count = static_cast<int>(get_minimum_load_offsets().size());
        for (int i = 0; i < loads_count; ++i) {
            String load_str = get_accepted_loads()[i]; // TypedArray<String> element
            float min_offset = 0.f;
            if (i < offsets_count) {
                // TypedArray<float> element may be represented as real Variant (double)
                min_offset = static_cast<float>(static_cast<double>(get_minimum_load_offsets()[i]));
            }
            p_mover->LoadAttributes.emplace_back(std::string(load_str.utf8().get_data()), min_offset);
        }
        p_mover->LoadQuantity = get_load_unit() == LOAD_UNIT_TONS ? "tons" : "pieces";
        p_mover->LoadSpeed = get_load_speed();
        p_mover->UnLoadSpeed = get_unload_speed();
        p_mover->OverLoadFactor = static_cast<float>(get_overload_factor());
        VehicleComponent::_do_update_internal_mover(p_mover);
    }


    void MoverVehicleLoad::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        VehicleComponent::_fill_config_dictionary(p_config);
    }


} // namespace godot
