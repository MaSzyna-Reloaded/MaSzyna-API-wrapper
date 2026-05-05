#pragma once

#include "E3DModel.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/visual_instance3d.hpp>

namespace godot {

    class E3DInstancer;

    class E3DModelInstance : public VisualInstance3D {
            GDCLASS(E3DModelInstance, VisualInstance3D)

        public:
            enum Instancer {
                OPTIMIZED = 0,
                NODES = 1,
                EDITABLE_NODES = 2,
            };

        private:
            Ref<E3DModel> model;
            Ref<E3DModel> loaded_model;
            bool dirty = false;
            bool e3d_loaded = false;
            E3DInstancer *current_instancer = nullptr;
            bool current_editable = false;
            String data_path;
            String model_filename;
            Array skins;
            Array exclude_node_names;
            Instancer instancer = Instancer::NODES;
            bool editable_in_editor = false;
            AABB submodels_aabb;

            E3DInstancer *_resolve_instancer() const;
            void _process_dirty();

        protected:
            static void _bind_methods();

        public:
            E3DModelInstance();

            void _notification(int p_what);
            AABB _get_aabb() const override;

            Ref<E3DModel> get_model() const;
            void set_model(const Ref<E3DModel> &p_model);

            String get_data_path() const;
            void set_data_path(const String &p_data_path);

            String get_model_filename() const;
            void set_model_filename(const String &p_model_filename);

            Array get_skins() const;
            void set_skins(const Array &p_skins);

            Array get_exclude_node_names() const;
            void set_exclude_node_names(const Array &p_exclude_node_names);

            Instancer get_instancer() const;
            void set_instancer(const Instancer p_instancer);

            bool is_editable_in_editor() const;
            void set_editable_in_editor(const bool p_editable_in_editor);

            void reload();
            bool is_e3d_loaded() const;
    };

} // namespace godot

VARIANT_ENUM_CAST(godot::E3DModelInstance::Instancer);
