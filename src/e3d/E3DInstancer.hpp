#pragma once

#include "E3DModel.hpp"
#include "materials/MaterialManager.hpp"
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/object.hpp>

namespace godot {

    class E3DModelInstance;

    class E3DInstancer : public Object {
            GDCLASS(E3DInstancer, Object)

        private:
            Ref<Material> colored_material;

        protected:
            static void _bind_methods();

            bool _is_submodel_valid(E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel) const;
            Ref<Material> _get_material_override(E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel);
            bool _requires_alpha_depth_prepass_sorting(const Ref<Material> &p_material) const;
            Ref<Material> _get_colored_material();

        public:
            virtual void instantiate(E3DModelInstance *p_target_node, const Ref<E3DModel> &p_model, bool p_editable);
            virtual void clear(E3DModelInstance *p_target_node);
            virtual void sync(E3DModelInstance *p_target_node);
    };

} // namespace godot
