#pragma once
#include "E3DModelInstanceManager.hpp"
#include "macros.hpp"
#include "models/e3d/E3DModel.hpp"

#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>

namespace godot {
    class E3DNodesInstancer; // forward declaration
    class E3DModelInstance: public VisualInstance3D {
        GDCLASS(E3DModelInstance, VisualInstance3D)
        public:
            static const char *e3d_loaded_signal;
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
            E3DModelInstanceManager *_get_manager() const;
            bool _is_dirty = false;
            bool _pending_model_scheduled = false;
            Ref<Mutex> _mutex;
            Ref<E3DModel> _pending_model;
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
            ~E3DModelInstance() override;
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
            
            void _instantiate_children(const Ref<E3DModel> &p_model);
    };
} //namespace godot

VARIANT_ENUM_CAST(E3DModelInstance::Instancer)
