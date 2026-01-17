#pragma once

#include "resources/material/MaszynaMaterial.hpp"
#include "settings/UserSettings.hpp"

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
    class MaterialParser;
    class MaterialManager : public Object {
        GDCLASS(MaterialManager, Object)

    public:
        enum Transparency {
            Disabled,
            Alpha,
            AlphaScissor
        };

    private:
        static MaterialParser *parser;
        static Ref<StandardMaterial3D> *_unknown_material;
        static Ref<ImageTexture> *_unknown_texture;
        static UserSettings *user_settings_node;
        static Dictionary *_textures;
        static Dictionary *_materials;

        static bool use_alpha_transparency;

        static const char* _transparency_codes[3];

        static void _clear_cache();
        static void _on_config_changed();

    protected:
        static void _bind_methods();

    public:
        static void init();
        static void cleanup();

        static String get_material_path(const String &model_name, const String &material_name);
        
        static Ref<MaszynaMaterial> load_material(const String &model_path, const String &material_name);
        static Ref<StandardMaterial3D> get_material(
            const String &model_path,
            const String &material_path,
            Transparency transparent = Disabled,
            bool is_sky = false,
            const Color &diffuse_color = Color(1.0, 1.0, 1.0)
        );
        static Ref<ImageTexture> get_texture(const String &texture_path);
        static Ref<ImageTexture> load_texture(const String &model_path, const String &material_name, bool global = true);
        static Ref<ImageTexture> load_submodel_texture(const String &model_path, const String &material_name);
    };

}

VARIANT_ENUM_CAST(godot::MaterialManager::Transparency);