#pragma once
#include "E3DInstanceBackend.hpp"
#include "E3DLightFactory.hpp"
#include "E3DNodesBackend.hpp"
#include "E3DOptimizedBackend.hpp"
#include "E3DSmokeSourceFactory.hpp"
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/particle_process_material.hpp>
#include <godot_cpp/classes/object.hpp>

namespace godot {
    /// RID based server of E3D model instances, similar to RenderingServer.
    /// The instancer of an instance selects how it is built: OPTIMIZED renders RenderingServer
    /// instances without any nodes, NODES/EDITABLE_NODES build a node tree under the attached node.
    /// RenderingServer instances are freed when the singleton is deleted.
    ///
    /// Scenery placements are registered with instance_register() instead of being built right
    /// away: SceneryStreamingServer builds them only while the camera is within their range.
    class E3DRenderingServer : public Object {
            GDCLASS(E3DRenderingServer, Object)

        public:
            enum Instancer {
                INSTANCER_OPTIMIZED,
                INSTANCER_NODES,
                INSTANCER_EDITABLE_NODES,
            };

            /// What a placement is: a static piece of the scenery (a "node ... model" of a .scn)
            /// against a dynamic one (its "dynamic", which is a vehicle). The original keeps the same distinction
            /// wherever it matters - a TAnimModel against a TDynamicObject, remembered by its
            /// particle emitters as owner_type { none, vehicle, node } (particles.h:135).
            /// Nothing about this is particular to smoke; smoke is only its first reader.
            enum InstanceKind {
                INSTANCE_KIND_STATIC,
                INSTANCE_KIND_DYNAMIC,
            };

            /// Light state of a scenery model node (TLightState, AnimModel.h:27-33)
            enum LightMode {
                LIGHT_MODE_OFF = 0,
                LIGHT_MODE_ON = 1,
                LIGHT_MODE_BLINK = 2,
                LIGHT_MODE_DARK = 3, // lit automatically once it gets dark
                LIGHT_MODE_HOME = 4, // like dark, but off late at night
            };

            /// Threshold of the light level below which a LIGHT_MODE_DARK light comes on, when the
            /// declared mode carries no fraction of its own (DefaultDarkThresholdLevel,
            /// AnimModel.h:24)
            static constexpr double DEFAULT_DARK_THRESHOLD = 0.325;
            /// LIGHT_MODE_HOME lights are forced off between these hours (AnimModel.cpp:601-607)
            static constexpr double HOME_LIGHTS_OFF_FROM_HOUR = 1.0;
            static constexpr double HOME_LIGHTS_OFF_TO_HOUR = 5.0;

            static constexpr const char *SCENERY_LIGHT_DISTANCE_SETTING = "maszyna/rendering/scenery_light_distance";
            static constexpr float DEFAULT_SCENERY_LIGHT_DISTANCE = 400.0;
            static constexpr const char *SCENERY_LIGHT_SHADOWS_SETTING = "maszyna/rendering/scenery_lights_shadows";
            static constexpr bool DEFAULT_SCENERY_LIGHT_SHADOWS = true;
            /// The original renders shadow maps with front faces culled (opengl33renderer.cpp:1634)
            static constexpr const char *LIGHTS_SHADOW_REVERSE_CULL_FACE_SETTING =
                    "maszyna/rendering/lights_shadow_reverse_cull_face";
            /// Shadows are dropped well before the light itself is, the way E3DNodesBackend fades
            /// a vehicle spotlight out
            static constexpr float SCENERY_LIGHT_SHADOW_FADE_DISTANCE = 80.0;
            /// A lamp must not shadow its own light. With one light per arm each arm was lit by its
            /// neighbours; economy mode leaves a single light in the middle, below which the arms
            /// and the pole throw long dark spokes right across the pool. The geometry of a model
            /// that carries a light goes on this layer as well as its own, and every scenery light
            /// leaves the layer out of its shadow caster mask - so the lamp still renders, is still
            /// lit, and still casts a shadow from the sun, just not into its own light.
            static constexpr uint32_t SCENERY_LIGHT_OWNER_LAYER = 1u << 19;
            /// A light created through RenderingServer starts with the server's own parameters,
            /// not with the ones SpotLight3D/OmniLight3D set in their constructors - without these
            /// the ground self-shadows into stripes. Same values a node would use.
            static constexpr float SPOT_LIGHT_SHADOW_BIAS = 0.03;
            static constexpr float OMNI_LIGHT_SHADOW_BIAS = 0.1;
            static constexpr float LIGHT_SHADOW_NORMAL_BIAS = 1.0;
            static constexpr const char *SCENERY_LIGHT_ENERGY_SETTING = "maszyna/rendering/scenery_light_energy";
            static constexpr float DEFAULT_SCENERY_LIGHT_ENERGY = 1.0;
            /// How much of the lamp's own colour is mixed into a white light. A sodium lamp's
            /// (1.0, 0.66, 0.18) used raw throws away most of the light's luminance and the pool
            /// comes out nearly black, so the colour tints white light instead of replacing it.
            static constexpr const char *SCENERY_LIGHT_TINT_SETTING = "maszyna/rendering/scenery_light_tint";
            static constexpr float DEFAULT_SCENERY_LIGHT_TINT = 0.5;
            static constexpr const char *SCENERY_LIGHT_VOLUMETRIC_FOG_ENERGY_SETTING =
                    "maszyna/rendering/scenery_light_volumetric_fog_energy";
            static constexpr float DEFAULT_SCENERY_LIGHT_VOLUMETRIC_FOG_ENERGY = 4.0;

            /// The original's gfx.smoke (Globals.cpp:1314). With it off no emitter is created at all.
            static constexpr const char *SMOKE_ENABLED_SETTING = "maszyna/rendering/smoke_enabled";
            static constexpr bool DEFAULT_SMOKE_ENABLED = true;
            /// A scenery emitter streams in at this distance. The original stops spawning beyond
            /// 2 * BaseDrawRange * fDistanceFactor (particles.cpp:452); a chimney has to be visible
            /// from further away than a street lamp, so this is not the scenery light distance.
            static constexpr const char *SMOKE_DISTANCE_SETTING = "maszyna/rendering/smoke_distance";
            static constexpr float DEFAULT_SMOKE_DISTANCE = 1500.0;
            /// The original displaces a particle by 0.1 * age * wind every step
            /// (particles.cpp:383), which integrates to 0.05 * wind * t^2 - exactly what a
            /// constant acceleration of 0.1 * wind gives, so the drift rides the process
            /// material's gravity.
            static constexpr float SMOKE_WIND_ACCELERATION = 0.1;
            /// Emitters visited per frame. Below this every emitter is visited every frame, which
            /// is what spreads its particles out evenly; above it the tick carries on where it
            /// left off, so an emitter waits a few frames and then spawns the whole backlog its
            /// own clock owes it. The count of particles is unchanged either way - only how
            /// evenly they are spread.
            static constexpr int MAX_SMOKE_SOURCES_PER_FRAME = 64;

        private:
            /// An addressable light of an instance. An emission light only switches the model's
            /// own light_onNN/light_offNN submodels (the backends do that from lights_state), a
            /// spot or omni light additionally owns a RenderingServer light.
            enum LightKind {
                LIGHT_KIND_EMISSION,
                LIGHT_KIND_SPOT,
                LIGHT_KIND_OMNI,
            };

            struct LightObject {
                    RID owner; // the E3D instance this light belongs to
                    LightKind kind = LIGHT_KIND_EMISSION;
                    String light_name; // the E3DModel.lights entry it follows
                    bool enabled = false;
                    bool synthesized = false; // added by the street lamp quirk
                    E3DLightParams params;
                    RID light;          // RenderingServer light
                    RID light_instance; // its RenderingServer instance
                    RID stream_rid;     // SceneryStreamingServer registration, scenery lights only
                    bool streamed_in = false;
            };

            /// A particle emitter of an instance. Unlike a light it exists for every instancer:
            /// a vehicle past maszyna/rendering/vehicle_detail_distance has no node tree left to
            /// hang one on, and its plume is the thing still visible at that range.
            struct SmokeObject {
                    RID owner; // the E3D instance this emitter belongs to
                    String template_name;
                    Vector3 offset; // relative to the model root
                    RID particles;
                    RID particles_instance;
                    Ref<ParticleProcessMaterial> process_material;
                    /// Reach of the plume around the emitter, in its own space; the particles are
                    /// left behind rather than carried, so the box follows the emitter and not the
                    /// plume - a fast vehicle's trail is culled with its emitter (see TODO.md)
                    AABB local_aabb;
                    float spawn_rate = 0.0;   // particles per second the template declares
                    float spawn_backlog = 0.0; // fractional particles carried to the next tick
                    uint64_t last_spawn_usec = 0; // its own clock, so a skipped frame costs nothing
                    int amount = 0;            // pool size, the cap on one tick's spawns
                    float intensity = 1.0;     // spawn rate multiplier
                    /// Mirrored from the owner instance whenever it moves or is shown/hidden, so
                    /// the per-frame tick is arithmetic on this struct alone and never looks an
                    /// instance up
                    Transform3D transform;
                    bool visible = true;
                    RID stream_rid;        // SceneryStreamingServer registration, scenery emitters only
                    bool streamed_in = false;
            };

            HashMap<RID, E3DInstanceData> instances;
            HashMap<RID, LightObject> lights;
            HashMap<RID, SmokeObject> smoke_objects;
            /// The same emitters in a flat list, so the per-frame tick walks a contiguous vector
            /// round-robin instead of a hash map
            Vector<RID> smoke_order;
            int smoke_cursor = 0;
            E3DOptimizedBackend optimized_backend;
            E3DNodesBackend nodes_backend{false};
            E3DNodesBackend editable_nodes_backend{true};
            E3DMaterialResolver material_resolver;

            /// What _stream_preload() needs off the main thread, where instances is not safe
            struct StreamModel {
                    String data_path;
                    String model_filename;
            };

            /// Registered instances are built through SceneryStreamingServer under this owner
            int stream_owner = -1;
            /// ...and the real lights of scenery instances under this one, with a range of their
            /// own: a street lamp is visible from half a kilometre and lights fifteen metres
            int light_stream_owner = -1;
            /// ...and the particle emitters under this one, with a range of their own again
            int smoke_stream_owner = -1;
            // Pushed by MaszynaEnvironmentNode, kept apart so either can be set on its own;
            // wind is the composed vector the emitters actually drift with, in m/s
            float wind_strength = 0.0;
            Vector3 wind_direction = Vector3(1.0, 0.0, 0.0);
            Vector3 wind;
            bool smoke_processing = false;
            double current_time = 12.0; // hours, 0..24
            double light_level = 1.0;   // Global.fLuminance equivalent (simulationenvironment.cpp:184)
            Callable model_loader;
            Callable smoke_source_resolver;
            HashMap<String, Ref<E3DModel>> models;
            HashMap<RID, StreamModel> stream_models;
            Ref<Mutex> models_mutex;

            E3DInstanceBackend &_get_backend(const E3DInstanceData &p_instance);
            void _rebuild_if_built(E3DInstanceData &p_instance);
            void _update_if_built(E3DInstanceData &p_instance);
            Ref<E3DModel> _load_model(const String &p_data_path, const String &p_model_filename);
            Variant _stream_preload(const RID &p_instance);
            void _stream_build(const RID &p_instance, const Variant &p_preloaded);
            void _stream_clear(const RID &p_instance);

            RID _light_create(
                    const RID &p_instance, const String &p_light_name, LightKind p_kind,
                    const E3DLightParams &p_params, bool p_synthesized = false);
            void _light_build(const RID &p_light);
            void _light_stream_build(const RID &p_light, const Variant &p_preloaded);
            void _light_clear(const RID &p_light);
            void _light_apply_enabled(LightObject &p_light);
            void _build_instance_lights(const RID &p_instance, E3DInstanceData &p_instance_data);
            static void _apply_declared_color(
                    const E3DInstanceData &p_instance_data, const String &p_light_name, E3DLightParams &p_params);
            void _clear_instance_lights(E3DInstanceData &p_instance_data);

            void _build_instance_smoke_sources(const RID &p_instance, E3DInstanceData &p_instance_data);
            void _smoke_build(const RID &p_smoke);
            void _smoke_stream_build(const RID &p_smoke, const Variant &p_preloaded);
            void _smoke_clear(const RID &p_smoke);
            void _clear_instance_smoke_sources(E3DInstanceData &p_instance_data);
            void _update_instance_smoke(const E3DInstanceData &p_instance_data);
            static Transform3D _smoke_transform(const E3DInstanceData &p_instance_data, const SmokeObject &p_smoke);
            static void _apply_smoke_placement(const E3DInstanceData &p_instance_data, SmokeObject &p_smoke);
            void _apply_smoke_wind(const SmokeObject &p_smoke) const;
            void _update_wind();
            /// Connected to SceneTree's process_frame while any emitter exists, the way
            /// SceneryStreamingServer drives its own streaming - no script runs per frame
            void _process_smoke();
            void _process_smoke_source(SmokeObject &p_smoke, uint64_t p_now);
            void _set_smoke_processing(bool p_processing);
            /// Resolves lights_state out of the declared modes, the manual overrides and the time
            /// of day, then applies it to the backend and to the instance's light objects
            void _resolve_lights(E3DInstanceData &p_instance);
            void _resolve_all_lights();
            bool _is_light_mode_on(float p_mode) const;
            static String _light_name_for_index(int p_index);

        protected:
            static void _bind_methods();

        public:
            E3DRenderingServer();
            ~E3DRenderingServer() override;

            RID instance_create(const Ref<E3DModel> &p_model, Instancer p_instancer, InstanceKind p_instance_kind);
            RID instance_register(
                    const String &p_data_path, const String &p_model_filename, const PackedStringArray &p_skins,
                    const Transform3D &p_transform, float p_range_begin, float p_range_end, const RID &p_scenario);
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
            /// The scenery node's `lights` list, by light index (light 0 is "00", AnimModel.cpp:303)
            void instance_set_lights_modes(const RID &p_instance, const PackedFloat32Array &p_modes);
            /// The scenery node's `lightcolors` list, in the same order
            void instance_set_lights_colors(const RID &p_instance, const PackedColorArray &p_colors);

            RID emission_light_create(const RID &p_instance, const String &p_light_name);
            RID spot_light_create(const RID &p_instance, const String &p_light_name, const NodePath &p_submodel_path);
            RID omni_light_create(const RID &p_instance, const String &p_light_name, const NodePath &p_submodel_path);
            void light_free(const RID &p_light);
            void light_enable(const RID &p_light);
            void light_disable(const RID &p_light);
            /// total/lit/spot/omni/synthesized, for the scenery streaming debug panel
            Dictionary get_light_statistics() const;

            /// Spawn rate multiplier of every emitter of the instance, as the engine state drives
            /// it. 1.0 is the template's own rate - what a scenery chimney keeps.
            ///
            /// Only the rate. Nothing here may reach a particle that is already in the air: both
            /// the process material's colour and its initial colour ramp are read every frame, so
            /// driving either of them from the engine state made the whole plume step down
            /// together instead of thinning out (see FINDINGS.md). The original scales the
            /// opacity of a particle by dizel_fill in its spawn routine (particles.cpp:330);
            /// Godot's only spawn-time channel is the emission itself, so dizel_fill is folded
            /// into the rate by the caller.
            void instance_set_smoke_intensity(const RID &p_instance, float p_intensity);
            /// total/built, for the scenery streaming debug panel
            Dictionary get_smoke_statistics() const;

            /// Pushed by MaszynaEnvironmentNode; the first two drive the automatic light modes,
            /// the wind drifts the particles of every emitter
            void set_current_time(double p_hours);
            void set_light_level(double p_level);
            void set_wind(float p_strength, const Vector3 &p_direction);
            void set_wind_strength(float p_strength);
            void set_wind_direction(const Vector3 &p_direction);

            void set_material_resolver(const Callable &p_material_resolver);
            void set_model_loader(const Callable &p_model_loader);
            void set_smoke_source_resolver(const Callable &p_smoke_source_resolver);
    };
} // namespace godot

VARIANT_ENUM_CAST(E3DRenderingServer::Instancer)
VARIANT_ENUM_CAST(E3DRenderingServer::LightMode)
VARIANT_ENUM_CAST(E3DRenderingServer::InstanceKind)
