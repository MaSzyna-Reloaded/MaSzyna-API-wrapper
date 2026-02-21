#pragma once

#include "MaterialParser.hpp"
#include "resources/material/MaszynaMaterial.hpp"

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    class MaterialManager : public Object {
            GDCLASS(MaterialManager, Object)
        private:
            String game_dir = "";
            Ref<Mutex> mutex;
            Ref<MaterialParser> parser;
            Ref<StandardMaterial3D> _unknown_material;
            Ref<ImageTexture> _unknown_texture;
            Object *user_settings_node = nullptr;
            Dictionary _materials;

            bool use_alpha_transparency = false;
            bool auto_generate_normal = false;
            bool auto_generate_metallic = false;
            bool auto_generate_height = false;

            const char *_transparency_codes[3] = {"0", "a", "s"};

            void _clear_cache();

        protected:
            static void _bind_methods();

        public:
            MaterialManager();
            ~MaterialManager() override;
            enum Transparency { Disabled, Alpha, AlphaScissor };
            String get_material_path(const String &model_name, const String &material_name);

            Ref<MaszynaMaterial> load_material(const String &model_path, const String &material_name);
            Ref<StandardMaterial3D> get_material(
                    const String &model_path, const String &material_path, Transparency transparent = Disabled,
                    bool is_sky = false, const Color &diffuse_color = Color(1.0, 1.0, 1.0));
            Ref<ImageTexture> get_texture(const String &texture_path);
            Ref<ImageTexture> load_texture(const String &model_path, const String &material_name);

            static Ref<Image> generate_heightmap_from_albedo(const Ref<Image> &p_albedo);
            static Ref<Image> generate_normal_from_albedo(const Ref<Image> &p_albedo, float p_strength = 1.0f);
            static Ref<Image> generate_metallic_from_albedo(const Ref<Image> &p_albedo, float p_threshold = 0.5f);
    };

} // namespace godot

VARIANT_ENUM_CAST(godot::MaterialManager::Transparency);
