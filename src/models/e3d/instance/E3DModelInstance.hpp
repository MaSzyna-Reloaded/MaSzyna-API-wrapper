#pragma once
#include "./core/utils.hpp"
#include "E3DModelInstanceManager.hpp"
#include "macros.hpp"
#include "models/e3d/E3DModel.hpp"

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include <vector>

namespace godot {
    class E3DNodesInstancer; // forward declaration
    class E3DModelInstance: public VisualInstance3D {
        GDCLASS(E3DModelInstance, VisualInstance3D)
        private:
            void _reload();
            void _deferred_reload();
            void _flush_pending_model();
            E3DModelInstanceManager *_get_manager() const;
            RID _get_current_scenario() const;
            void _clear_optimized_instances();
            void _sync_optimized_instances();
            void _apply_material_overrides_to_node(Node *node) const;
            bool _is_dirty = false;
            bool _pending_model_scheduled = false;
            Ref<Mutex> _mutex;
            Ref<E3DModel> _pending_model;
            Ref<E3DModel> _loaded_model;
            Dictionary _submodel_material_overrides;
            struct OptimizedInstanceRecord {
                RID instance_rid;
                Transform3D local_transform;
                bool visible;
                String name;
            };
            std::vector<OptimizedInstanceRecord> _optimized_instances;
        protected:
            static void _bind_methods();
        public:
            static const char *E3D_LOADED_SIGNAL;
            enum Instancer {
                INSTANCER_OPTIMIZED,
                INSTANCER_NODES,
                INSTANCER_EDITABLE_NODES,
            };

            //@TODO: Maybe remove this?
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
            MAKE_MEMBER_GS(AABB, submodels_aabb, AABB())
            MAKE_MEMBER_OBSERVABLE_GS(bool, editable_in_editor)
            
            AABB _get_aabb() const override;
            void _instantiate_children(const Ref<E3DModel> &p_model);
            void set_submodel_material_override(const String &submodel_name, const Ref<Material> &material);
            Ref<Material> get_submodel_material_override(const String &submodel_name) const;
            void clear_submodel_material_override(const String &submodel_name);
    };
} //namespace godot

VARIANT_ENUM_CAST(E3DModelInstance::Instancer)
