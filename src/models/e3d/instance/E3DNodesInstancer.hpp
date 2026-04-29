#pragma once
#include "models/e3d/E3DModel.hpp"
#include "models/e3d/instance/E3DModelInstance.hpp"
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    class E3DNodesInstancer : public Object {
            GDCLASS(E3DNodesInstancer, Object)
        private:
            Ref<Material> colored_material;
            Node3D *
            _create_submodel_instance(const E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel);
            void _do_add_submodels(
                    E3DModelInstance *p_target_node, Node3D *p_parent, const TypedArray<E3DSubModel> &p_submodels,
                    bool p_editable);
            void _update_submodel_material(
                    const E3DModelInstance *p_target_node, Node3D *p_subnode, const Ref<E3DSubModel> &p_submodel);

        protected:
            static void _bind_methods();

        public:
            static E3DNodesInstancer *get_instance() {
                return dynamic_cast<E3DNodesInstancer *>(Engine::get_singleton()->get_singleton("E3DNodesInstancer"));
            }

            void cleanup();
            Ref<Material> get_colored_material();
            void instantiate(const Ref<E3DModel> &p_model, E3DModelInstance *p_target_node, bool p_editable = false);
            void clear_children(E3DModelInstance *p_target_node, bool p_immediate = false);
            Ref<Material>
            resolve_submodel_material(const E3DModelInstance *p_target_node, const Ref<E3DSubModel> &p_submodel);
    };
} // namespace godot
