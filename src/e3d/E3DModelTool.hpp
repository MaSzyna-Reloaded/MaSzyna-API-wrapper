#pragma once

#include "E3DModel.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    class E3DModelTool : public Object {
            GDCLASS(E3DModelTool, Object)

        private:
            Ref<ArrayMesh> _find_first_mesh_in_submodels(const Array &p_submodels) const;
            bool _get_aabb_from_submodel(
                    const Ref<E3DSubModel> &p_submodel, const Transform3D &p_parent_transform, AABB &r_aabb,
                    bool &r_has_aabb) const;

        protected:
            static void _bind_methods();

        public:
            static E3DModelTool *get_instance() {
                return dynamic_cast<E3DModelTool *>(Engine::get_singleton()->get_singleton("E3DModelTool"));
            }

            Ref<ArrayMesh> find_first_mesh(const Ref<E3DModel> &p_model) const;
            AABB get_aabb(const Ref<E3DModel> &p_model) const;
    };

} // namespace godot
