#pragma once
#include "E3DInstanceBackend.hpp"
#include "E3DNodesBackend.hpp"
#include "E3DOptimizedBackend.hpp"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/object.hpp>

namespace godot {
    /// RID based server of E3D model instances, similar to RenderingServer.
    /// The instancer of an instance selects how it is built: OPTIMIZED renders RenderingServer
    /// instances without any nodes, NODES/EDITABLE_NODES build a node tree under the attached node.
    /// RenderingServer instances are freed when the singleton is deleted.
    class E3DRenderingServer : public Object {
            GDCLASS(E3DRenderingServer, Object)

        public:
            enum Instancer {
                INSTANCER_OPTIMIZED,
                INSTANCER_NODES,
                INSTANCER_EDITABLE_NODES,
            };

        private:
            HashMap<RID, E3DInstanceData> instances;
            E3DOptimizedBackend optimized_backend;
            E3DNodesBackend nodes_backend{false};
            E3DNodesBackend editable_nodes_backend{true};
            E3DMaterialResolver material_resolver;

            E3DInstanceBackend &_get_backend(const E3DInstanceData &p_instance);
            void _rebuild_if_built(E3DInstanceData &p_instance);
            void _update_if_built(E3DInstanceData &p_instance);

        protected:
            static void _bind_methods();

        public:
            ~E3DRenderingServer() override;

            RID instance_create(const Ref<E3DModel> &p_model, Instancer p_instancer);
            void instance_free(const RID &p_instance);
            void instance_build(const RID &p_instance);
            void instance_set_options(
                    const RID &p_instance, const String &p_data_path, const PackedStringArray &p_skins,
                    const Array &p_exclude_node_names, bool p_force_alpha,
                    const TypedArray<NodePath> &p_force_alpha_submodel_paths);
            void instance_attach_node(const RID &p_instance, Node3D *p_node);
            void instance_set_scenario(const RID &p_instance, const RID &p_scenario);
            void instance_set_transform(const RID &p_instance, const Transform3D &p_transform);
            void instance_set_visible(const RID &p_instance, bool p_visible);
            void instance_set_layer_mask(const RID &p_instance, uint32_t p_mask);
            void instance_set_visibility_range(const RID &p_instance, float p_begin, float p_end);
            void instance_set_lights_state(const RID &p_instance, const Dictionary &p_lights_state);

            void set_material_resolver(const Callable &p_material_resolver);
    };
} // namespace godot

VARIANT_ENUM_CAST(E3DRenderingServer::Instancer)
