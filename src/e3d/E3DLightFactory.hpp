#pragma once
#include "E3DModel.hpp"
#include "E3DSubModel.hpp"
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {
    /// Light parameters derived from a model, shared by every backend that turns an E3D light into
    /// a real one (a SpotLight3D node or a RenderingServer light RID).
    struct E3DLightParams {
            /// Godot's spot cone is capped below 90 degrees, so a wider one has to be an omni
            bool omni = false;
            Transform3D transform; // relative to the model root
            Color color = Color(1.0, 1.0, 1.0);
            float energy = 0.0;
            float range = 0.0;
            float spot_angle = 0.0;
            float spot_attenuation = 1.0;
            float attenuation = 1.0;
    };

    /// One of the model's lights: the submodel pair it shows and hides
    struct E3DModelLight {
            String name;
            E3DSubModel *on = nullptr;
            E3DSubModel *off = nullptr;
    };

    /// A real light the model carries, already placed relative to the model root
    struct E3DModelLightPlacement {
            String light_name; // the E3DModelLight it is switched with
            E3DLightParams params;
            bool synthesized = false; // added by the street lamp quirk, not declared by the model
    };

    /// Everything about a model's lights, found once and consumed by the backends and by
    /// E3DRenderingServer. Identifying lights is not an instancer's job - both instancers only
    /// render what is listed here.
    struct E3DModelLights {
            Vector<E3DModelLight> lights;
            Vector<E3DModelLightPlacement> placements;

            const E3DModelLight *find(const String &p_name) const;
    };

    /// Builds E3DLightParams out of the data an E3D model carries.
    class E3DLightFactory {
        public:
            /// Walks the model once: pairs every light_onNN with its light_offNN, collects the
            /// SUBMODEL_FREE_SPOTLIGHTs with the light of their nearest "on" ancestor and their
            /// transform relative to the model root, and adds the street lamp quirk for models
            /// that light the scene without declaring a spotlight at all.
            static E3DModelLights discover(const Ref<E3DModel> &p_model, const String &p_model_filename);


            /// Godot rejects a spot cone of 90 degrees or more (Light3D::PARAM_SPOT_ANGLE).
            /// Of the 871 FREE_SPOTLIGHT submodels in the data set 6 are wider, among them
            /// elektryczne/lampa_parkowa01 at 117 degrees - those become omni lights.
            static constexpr float MAX_SPOT_ANGLE = 89.9;

            static constexpr float DEFAULT_LIGHT_ENERGY = 0.8;
            static constexpr float DEFAULT_HIGHBEAM_LIGHT_ENERGY = 2.0;
            static constexpr float DEFAULT_HEAD_LIGHT_ENERGY = 1.0;
            static constexpr float DEFAULT_END_LIGHT_ENERGY = 1.0;
            static constexpr float DEFAULT_LIGHT_SPOT_RANGE = 40.0;
            static constexpr float FORCED_END_LIGHT_SPOT_RANGE = 3.0;

            /// Parameters of a SUBMODEL_FREE_SPOTLIGHT, as the model declares them. The light name
            /// selects the vehicle energy/range overrides (headlamp, highbeam, end signal).
            static E3DLightParams from_submodel(E3DSubModel *p_submodel, const String &p_light_name);

            /// Street lamps (models named "latarnia*") declare no spotlight submodel at all - none
            /// of the 9 in the data set has one, and the original never lit the scene with them
            /// either (TP_FREESPOTLIGHT only draws a glare billboard, opengl33renderer.cpp:4646).
            /// The lamp head and the lit patch are still modelled, as billboards under light_onNN:
            /// the halo carries the only non-identity matrix in the model (the head), and the
            /// ground patch quad spans the area the lamp is meant to light. Both are found by
            /// their material, not by name - latarnial_str calls its halos pos11/pos22/pos33 while
            /// the rest call them plane02/plane04/plane06.
            /// Returns false when either anchor is missing, rather than inventing a light.
            static bool make_street_lamp(
                    E3DSubModel *p_on_submodel, const Transform3D &p_on_transform, E3DLightParams &p_params);

            /// True for the models the street lamp quirk applies to
            static bool is_street_lamp(const String &p_model_filename);
    };
} // namespace godot
