#pragma once
#include "E3DInstanceBackend.hpp"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/spot_light3d.hpp>

namespace godot {
    /// Builds submodels as a Node3D/MeshInstance3D/SpotLight3D tree under the attached node.
    /// (formerly e3d_nodes_instancer.gd)
    class E3DNodesBackend : public E3DInstanceBackend {
        private:
            bool editable = false; // generated nodes stay editable in the editor (not internal)

            static constexpr float DEFAULT_LIGHT_ENERGY = 0.8;
            static constexpr float DEFAULT_HIGHBEAM_LIGHT_ENERGY = 2.0;
            static constexpr float DEFAULT_HEAD_LIGHT_ENERGY = 1.0;
            static constexpr float DEFAULT_END_LIGHT_ENERGY = 1.0;
            static constexpr float DEFAULT_LIGHT_SPOT_RANGE = 40.0;
            static constexpr float FORCED_END_LIGHT_SPOT_RANGE = 3.0;

            struct LightRole {
                    String light_name;
                    bool on = false;
            };

            void _add_submodels(
                    E3DInstanceData &p_instance, Node3D *p_target, Node3D *p_parent,
                    const TypedArray<E3DSubModel> &p_submodels, const HashMap<E3DSubModel *, LightRole> &p_light_roles,
                    const String &p_parent_light_name, const Vector<E3DSubModel *> &p_force_alpha_submodels,
                    bool p_force_alpha, const Callable &p_material_resolver);
            static Node3D *_create_submodel_node(E3DSubModel *p_submodel);
            static void
            _configure_spotlight(SpotLight3D *p_spotlight, const String &p_light_name, E3DSubModel *p_submodel);
            static void _set_node_visible(const ObjectID &p_node_id, bool p_visible);

        public:
            explicit E3DNodesBackend(bool p_editable);

            void build(E3DInstanceData &p_instance, const Callable &p_material_resolver) override;
            void clear(E3DInstanceData &p_instance) override;
            void update(const E3DInstanceData &p_instance) override;
    };
} // namespace godot
