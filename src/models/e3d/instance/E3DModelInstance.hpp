#pragma once
#include "macros.hpp"
#include "./core/utils.hpp"
#include "models/e3d/E3DModel.hpp"

#include <godot_cpp/classes/visual_instance3d.hpp>

namespace godot {
    class E3DModelInstanceManager; // forward declaration
    class E3DModelInstance: public VisualInstance3D {
        GDCLASS(E3DModelInstance, VisualInstance3D)
        private:
            void _reload();
            E3DModelInstanceManager *_manager;
        protected:
            static void _bind_methods();
        public:
            enum Instancer {
                INSTANCER_OPTIMIZED,
                INSTANCER_NODES,
                INSTANCER_EDITABLE_NODES,
            };
            void _notification(int p_what);
            E3DModelInstance();
            ~E3DModelInstance() override;
            MAKE_MEMBER_GS(Vector3, default_aabb_size, Vector3(1, 1, 1))
            // Observable values that trigger _reload() when changed
            MAKE_MEMBER_OBSERVABLE_GS(String, data_path);
            MAKE_MEMBER_OBSERVABLE_GS(String, model_filename);
            MAKE_MEMBER_OBSERVABLE_GS(Array, skins);
            MAKE_MEMBER_OBSERVABLE_GS(Array, exclude_node_names);
            MAKE_MEMBER_OBSERVABLE_GS_NR(Instancer, instancer)
            MAKE_MEMBER_OBSERVABLE_GS(AABB, submodels_aabb)
            MAKE_MEMBER_OBSERVABLE_GS(bool, editable_in_editor)
    };
} //namespace godot

VARIANT_ENUM_CAST(E3DModelInstance::Instancer)
