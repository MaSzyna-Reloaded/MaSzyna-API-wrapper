#pragma once
#include "E3DLightFactory.hpp"
#include "E3DMaterialResolver.hpp"
#include "E3DModel.hpp"
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /// State of a single E3DRenderingServer instance. Settings are written by the server,
    /// the rest belongs to the backend that built the instance.
    struct E3DInstanceData {
            struct LightSubmodels {
                    E3DSubModel *on = nullptr;
                    E3DSubModel *off = nullptr;
            };

            /// A scenery node's `lights`/`lightcolors` entry for one light
            /// (TAnimModel::Load(), AnimModel.cpp:335-361)
            struct LightDeclaration {
                    float mode = 0.0; // ls_Off/ls_On/ls_Blink/ls_Dark/ls_Home plus its fraction
                    Color color;
                    bool has_color = false;
            };

            struct LightNodes {
                    ObjectID on;
                    ObjectID off;
                    ObjectID spotlight;
                    E3DSubModel *submodel = nullptr; // first matched on/off submodel, configures the spotlight
            };

            Ref<E3DModel> model; // keeps submodels and meshes alive
            /// The model's lights, found by E3DLightFactory before the backend builds
            E3DModelLights model_lights;
            int instancer = 0;
            bool built = false;
            RID stream_rid;        // valid when registered with SceneryStreamingServer
            String model_filename; // set by instance_register(), loaded when it comes in range

            String data_path;
            PackedStringArray skins;
            Array exclude_node_names;
            bool force_alpha = false;
            TypedArray<NodePath> force_alpha_submodel_paths;
            ObjectID node_id;
            RID scenario;
            Transform3D transform;
            bool visible = true;
            uint32_t layer_mask = 1;
            float visibility_range_begin = 0.0;
            float visibility_range_end = 0.0;
            /// Light name -> enabled, resolved by E3DRenderingServer out of light_declarations,
            /// lights_override and the time of day; this is what the backends read
            Dictionary lights_state;
            /// What the scenery node declared, kept across a stream clear/build cycle
            HashMap<String, LightDeclaration> light_declarations;
            /// Set through instance_set_lights_state(); wins over the declared mode
            Dictionary lights_override;
            /// Real lights owned by this instance (E3DRenderingServer light RIDs)
            Vector<RID> light_objects;
            /// Particle emitters owned by this instance (E3DRenderingServer smoke RIDs)
            Vector<RID> smoke_objects;

            // OPTIMIZED backend
            HashMap<String, LightSubmodels> light_submodels;
            Vector<RID> rids;
            Vector<Transform3D> local_transforms;
            Vector<Vector<E3DSubModel *>> chains; // submodel with all its ancestors
            Vector<Ref<Material>> materials;

            // NODES backend
            Vector<ObjectID> root_nodes;
            HashMap<String, LightNodes> light_nodes;
    };

    /// Builds and updates the content of E3DRenderingServer instances.
    class E3DInstanceBackend {
        public:
            virtual ~E3DInstanceBackend() = default;

            virtual void build(E3DInstanceData &p_instance, E3DMaterialResolver &p_material_resolver) = 0;
            virtual void clear(E3DInstanceData &p_instance) = 0;
            /// Applies transform, visibility, layers and lights state
            virtual void update(const E3DInstanceData &p_instance) = 0;

        protected:
            static bool _is_submodel_valid(const E3DSubModel *p_submodel, const Array &p_exclude_node_names);
            static Vector<E3DSubModel *> _get_force_alpha_submodels(const E3DInstanceData &p_instance);
            static bool _is_force_alpha(
                    const E3DInstanceData &p_instance, E3DSubModel *p_submodel,
                    const Vector<E3DSubModel *> &p_force_alpha_submodels, bool p_parent_force_alpha);
            static bool _requires_alpha_depth_prepass_sorting(const Ref<Material> &p_material);
    };
} // namespace godot
