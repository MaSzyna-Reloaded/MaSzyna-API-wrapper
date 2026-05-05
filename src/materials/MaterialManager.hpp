#pragma once

#include "MaterialParser.hpp"
#include "core/ResourceCache.hpp"
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>

namespace godot {

    class MaterialManager : public Object {
            GDCLASS(MaterialManager, Object)

        public:
            enum Transparency {
                DISABLED = 0,
                ALPHA = 1,
                ALPHA_SCISSOR = 2,
            };

        private:
            Ref<ResourceCache> materials_cache;
            Ref<Texture2D> unknown_texture;
            bool use_alpha_transparency = false;

            String _get_transparency_code(const Transparency p_transparency) const;
            Ref<MaszynaMaterial> load_material(const String &p_model_path, const String &p_material_name) const;
            Ref<Texture2D>
            load_texture(const String &p_model_path, const String &p_material_name, const bool p_normal = false) const;

        protected:
            static void _bind_methods();

        public:
            MaterialManager();

            static MaterialManager *get_instance() {
                return dynamic_cast<MaterialManager *>(Engine::get_singleton()->get_singleton("MaterialManager"));
            }

            void clear_cache();
            Ref<StandardMaterial3D> get_material(
                    const String &p_model_path, const String &p_material_path,
                    const Transparency p_transparent = Transparency::DISABLED, const bool p_is_sky = false,
                    const Color &p_diffuse_color = Color(1.0, 1.0, 1.0)) const;
            Ref<Texture2D> get_texture(const String &p_texture_path) const;
    };

} // namespace godot

VARIANT_ENUM_CAST(godot::MaterialManager::Transparency);
