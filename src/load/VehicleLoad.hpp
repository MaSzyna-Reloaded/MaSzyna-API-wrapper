#pragma once
#include "../core/VehicleComponent.hpp"
#include "../macros.hpp"
#include "../resources/load/LoadListItem.hpp"

namespace godot {
    class VehicleLoad : public VehicleComponent {
            GDCLASS(VehicleLoad, VehicleComponent)

        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_LOAD;
            }

        private:
            static void _bind_methods();
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            enum LoadUnit { LOAD_UNIT_TONS, LOAD_UNIT_PIECES };
            MAKE_MEMBER_GS_NR(LoadUnit, load_unit, LoadUnit::LOAD_UNIT_TONS);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<String>, accepted_loads);
            MAKE_MEMBER_GS_NR_NO_DEF(TypedArray<float>, minimum_load_offsets)
            MAKE_MEMBER_GS(float, max_load, 0.0f);
            MAKE_MEMBER_GS(double, overload_factor, 0.0f);
            MAKE_MEMBER_GS(float, load_speed, 0.0f);
            MAKE_MEMBER_GS(float, unload_speed, 0.0f);
            virtual void set_load_list(const TypedArray<LoadListItem> &p_load_list) = 0;
            virtual TypedArray<LoadListItem> get_load_list() = 0;
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleLoad::LoadUnit)
