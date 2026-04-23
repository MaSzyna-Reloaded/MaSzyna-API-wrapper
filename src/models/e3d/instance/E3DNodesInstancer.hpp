#pragma once
#include "models/e3d/instance/E3DModelInstance.hpp"
#include "models/e3d/E3DModel.hpp"
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

namespace godot {
    class E3DNodesInstancer : public Node {
        GDCLASS(E3DNodesInstancer, Node)
        private:
            static Ref<Material> colored_material;
            static Node3D *_create_submodel_instance(const E3DModelInstance &p_target_node, const E3DSubModel &p_submodel);
            static void _do_add_submodels(
                    const E3DModelInstance &p_target_node, Node3D *p_parent, const TypedArray<E3DSubModel> &p_submodels,
                    bool p_editable);
            static void _update_submodel_material(const E3DModelInstance &p_target_node, Node3D &p_subnode, const E3DSubModel &p_submodel);
        protected:
            static void _bind_methods();
        public:
            static void cleanup();
            static Ref<Material> get_colored_material();
            static void instantiate(const Ref<E3DModel> &p_model, E3DModelInstance *p_target_node, bool p_editable = false);
            static void clear_children(E3DModelInstance *p_target_node);
            static Ref<Material> resolve_submodel_material(const E3DModelInstance &p_target_node, const E3DSubModel &p_submodel);
    };
} //namespace godot
