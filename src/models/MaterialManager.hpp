#pragma once

#include "MaterialParser.hpp"
#include "core/ResourceCache.hpp"
#include "macros.hpp"
#include "resources/material/MaszynaMaterial.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    class MaterialManager : public Object {
            GDCLASS(MaterialManager, Object)
        private:
            Ref<Mutex> mutex;
            Ref<MaterialParser> parser;
            Ref<StandardMaterial3D> _unknown_material;
            Ref<ResourceCache> textures_cache;
            Ref<ImageTexture> _unknown_texture;
            Dictionary _materials;

            bool use_alpha_transparency = false;
            bool auto_generate_normal = false;
            bool auto_generate_metallic = false;
            bool auto_generate_height = false;

            const char *_transparency_codes[3] = {"0", "a", "s"};

        protected:
            static void _bind_methods();

        public:
            static MaterialManager *get_instance() {
                Engine *engine = Engine::get_singleton();
                if (engine == nullptr) {
                    return nullptr;
                }
                return Object::cast_to<MaterialManager>(engine->get_singleton("MaterialManager"));
            }

            MaterialManager();
            ~MaterialManager() override;
            enum Transparency { DISABLED, ALPHA, ALPHA_SCISSOR };
            void clear_cache();
            String get_material_path(const String &p_model_name, const String &p_material_name);

            Ref<MaszynaMaterial> load_material(const String &p_model_path, const String &p_material_name);
            Ref<StandardMaterial3D> get_material(
                    const String &p_model_path, const String &p_material_path, Transparency p_transparent = DISABLED,
                    bool p_is_sky = false, const Color &p_diffuse_color = Color(1.0, 1.0, 1.0));
            Ref<ImageTexture> get_texture(const String &p_texture_path);
            Ref<ImageTexture> load_texture(const String &p_model_path, const String &p_material_name);

            static Ref<Image> generate_heightmap_from_albedo(const Ref<Image> &p_albedo);
            static Ref<Image> generate_normal_from_albedo(const Ref<Image> &p_albedo, float p_strength = 1.0f);
            static Ref<Image> generate_metallic_from_albedo(const Ref<Image> &p_albedo, float p_threshold = 0.5f);
    };
} // namespace godot

VARIANT_ENUM_CAST(MaterialManager::Transparency);
