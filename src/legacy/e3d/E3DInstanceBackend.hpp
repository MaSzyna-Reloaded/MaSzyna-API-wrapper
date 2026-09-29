#pragma once
#include "E3DLightFactory.hpp"
#include "E3DMaterialResolver.hpp"
#include "E3DModel.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
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
                    E3DSubModel *xon = nullptr;
            };

            /// A scenery node's `lights`/`lightcolors` entry for one light
            /// (TAnimModel::Load(), AnimModel.cpp:335-361)
            struct LightDeclaration {
                    int mode = 0;          // E3DRenderingServer::LightMode
                    float threshold = 0.0; // LIGHT_MODE_DARK/HOME: light level it comes on at, 0 - default
                    float on_time = 0.0;   // LIGHT_MODE_BLINK: seconds on, seconds off, shift of the cycle
                    float off_time = 0.0;
                    float phase = 0.0;
                    bool blink_on = false; // LIGHT_MODE_BLINK: what the last resolve decided
                    Color color;
                    bool has_color = false;
            };

            /// A scenery `animation` event's motion of one submodel, towards its target at a speed
            /// in degrees or metres per second (TAnimContainer, AnimModel.cpp:51-188)
            struct SubmodelAnimation {
                    Vector3 angles; // degrees about the submodel's own x, y, z (TSubModel::SetRotateXYZ)
                    Vector3 target_angles;
                    double rotate_speed = 0.0;
                    Vector3 offset; // metres (TSubModel::SetTranslate)
                    Vector3 target_offset;
                    double translate_speed = 0.0;
                    /// The animated submodel of the built model, found once per build
                    E3DSubModel *submodel = nullptr;
            };

            struct LightNodes {
                    ObjectID on;
                    ObjectID off;
                    ObjectID xon;
                    ObjectID spotlight;
                    float spotlight_energy = 0.0;    // undimmed, as _configure_spotlight() set it
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
            /// Light name -> dimmed (instance_set_lights_dimmed()): its "_xon" submodel is shown instead
            /// of "_on" if it has one, and its real light shines at lights_dimmed_multiplier
            Dictionary lights_dimmed;
            float lights_dimmed_multiplier = 1.0;
            /// Real lights owned by this instance (E3DRenderingServer light RIDs)
            Vector<RID> light_objects;
            /// By lower-case submodel name, kept across a stream clear/build cycle
            HashMap<String, SubmodelAnimation> submodel_animations;
            /// What the animations make of their submodels, on top of the submodel's own transform;
            /// what the backends read
            HashMap<E3DSubModel *, Transform3D> submodel_poses;
            /// Particle emitters owned by this instance (E3DRenderingServer smoke RIDs)
            Vector<RID> smoke_objects;
            /// Spawn rate multiplier of those emitters (instance_set_smoke_intensity()), kept here so
            /// a rebuild gives the new emitters the rate the client last set
            float smoke_intensity = 1.0;
            /// Drawn over every submodel (instance_set_material_overlay()), kept across a stream
            /// clear/build cycle
            Ref<Material> material_overlay;
            /// E3DRenderingServer::InstanceKind - scenery unless the client says otherwise,
            /// which is what a placement registered for streaming always is
            int instance_kind = 0;

            // OPTIMIZED backend
            HashMap<String, LightSubmodels> light_submodels;
            Vector<RID> rids;
            Vector<Transform3D> local_transforms;
            Vector<Vector<E3DSubModel *>> chains; // submodel with all its ancestors
            Vector<Ref<Material>> materials;

            // NODES backend
            Vector<ObjectID> root_nodes;
            HashMap<String, LightNodes> light_nodes;
            HashMap<E3DSubModel *, ObjectID> submodel_nodes;
    };

    /// Builds and updates the content of E3DRenderingServer instances.
    class E3DInstanceBackend {
        public:
            /// What a light shows: its "_on", its "_xon" while dimmed (the "_on" when it has none -
            /// TButton::TurnxOnWithOnAsFallback(), DynObj.cpp:1218), its "_off" while out
            enum LightPart { LIGHT_PART_ON, LIGHT_PART_OFF, LIGHT_PART_XON };

            E3DInstanceBackend();
            virtual ~E3DInstanceBackend() = default;

            virtual void build(E3DInstanceData &p_instance, E3DMaterialResolver &p_material_resolver) = 0;
            virtual void clear(E3DInstanceData &p_instance) = 0;
            /// Applies transform, visibility, layers and lights state
            virtual void update(const E3DInstanceData &p_instance) = 0;
            /* Moves what was built, and nothing else. A moving instance is the per-frame path, so
             * it must not re-apply the lights state: the submodels a light switches are shown and
             * hidden by whoever owns that light, and re-applying them on every move overwrites
             * that owner once per frame (see `FINDINGS.md`, 2026-09-23). */
            virtual void apply_transform(const E3DInstanceData &p_instance) = 0;
            /// Places the submodels as submodel_poses says; called when an animation moved them
            virtual void apply_poses(E3DInstanceData &p_instance) = 0;
            /// The nearest hit of the segment (world space) on the built meshes nearer than
            /// `r_distance`: updates `r_distance` and `r_point` and returns true
            virtual bool intersect_segment(
                    const E3DInstanceData &p_instance, const Vector3 &p_from, const Vector3 &p_to, double &p_r_distance,
                    Vector3 &p_r_point) const = 0;

        protected:
            /// intersect_segment() of one mesh placed by `p_transform`
            static bool _intersect_mesh(
                    const Ref<Mesh> &p_mesh, const Transform3D &p_transform, const Vector3 &p_from, const Vector3 &p_to,
                    double &p_r_distance, Vector3 &p_r_point);
            /// Whether a light's part is shown under its state and dimming
            static bool _light_part_visible(
                    const E3DInstanceData &p_instance, const String &p_light_name, LightPart p_part, bool p_has_xon);
            /// The unit quad a free spotlight's point and glare are drawn with - the material's shaders
            /// place it on the screen (types/free_spotlight.gdshader)
            Ref<ArrayMesh> point_mesh;

            /// The instance shader parameters of a free spotlight's point, by name
            static Dictionary _free_spotlight_parameters(
                    const E3DInstanceData &p_instance, const E3DSubModel *p_submodel, const String &p_light_name);
            static bool _is_submodel_valid(const E3DSubModel *p_submodel, const Array &p_exclude_node_names);
            /// The submodel's own visibility in this instance: a "_on" control is hidden by default
            /// only in a dynamic (vehicle) model (Model3d.cpp:275, 2221)
            static bool _is_submodel_shown(const E3DInstanceData &p_instance, const E3DSubModel *p_submodel);
            static Vector<E3DSubModel *> _get_force_alpha_submodels(const E3DInstanceData &p_instance);
            static bool _is_force_alpha(
                    const E3DInstanceData &p_instance, E3DSubModel *p_submodel,
                    const Vector<E3DSubModel *> &p_force_alpha_submodels, bool p_parent_force_alpha);
            static bool _requires_alpha_depth_prepass_sorting(const Ref<Material> &p_material);
    };
} // namespace godot
