#pragma once
#include "macros.hpp"
#include "models/e3d/E3DModel.hpp"

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include <vector>

namespace godot {
    class E3DModelInstance : public VisualInstance3D {
            GDCLASS(E3DModelInstance, VisualInstance3D)
        public:
            static const char *e3d_loaded_signal;
            static const char *reloaded_signal;
            enum Instancer {
                INSTANCER_OPTIMIZED,
                INSTANCER_NODES,
                INSTANCER_EDITABLE_NODES,
            };

        private:
            void _reload();
            void _request_reload();
            void _deferred_reload();
            void _flush_pending_model();
            void _reset_reload_state();
            void _finish_extension_reload();

            void _clear_optimized_instances();
            void _sync_optimized_instances();
            void _collect_optimized_submodels(
                    const TypedArray<E3DSubModel> &p_submodels, const Transform3D &p_parent_transform,
                    bool p_parent_visible);

            bool _is_dirty = false;
            bool _is_tearing_down = false;
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

            String data_path;
            String model_filename;
            Array skins;
            Array exclude_node_names;
            Instancer instancer = INSTANCER_NODES;
            AABB submodels_aabb;
            bool editable_in_editor = false;

        protected:
            static void _bind_methods();

        public:
            //@TODO: Maybe remove this?
            void _notification(int p_what);
            E3DModelInstance();
            void reload();
            MAKE_MEMBER_GS(Vector3, default_aabb_size, Vector3(1, 1, 1))
            String get_data_path() const;
            void set_data_path(const String &p_value);
            String get_model_filename() const;
            void set_model_filename(const String &p_value);
            Array get_skins() const;
            void set_skins(const Array &p_value);
            Array get_exclude_node_names() const;
            void set_exclude_node_names(const Array &p_value);
            Instancer get_instancer() const;
            void set_instancer(Instancer p_value);
            AABB get_submodels_aabb() const;
            void set_submodels_aabb(const AABB &p_value);
            bool get_editable_in_editor() const;
            void set_editable_in_editor(const bool &p_value);

            AABB _get_aabb() const override;
            void _instantiate_children(const Ref<E3DModel> &p_model);
            void cleanup_for_extension_reload();
            void set_submodel_material_override(const String &submodel_name, const Ref<Material> &material);
            Ref<Material> get_submodel_material_override(const String &submodel_name) const;
            void clear_submodel_material_override(const String &submodel_name);
    };
} // namespace godot

VARIANT_ENUM_CAST(E3DModelInstance::Instancer)
