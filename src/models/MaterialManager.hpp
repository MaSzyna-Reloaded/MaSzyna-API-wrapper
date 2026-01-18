#pragma once

#include "resources/material/MaszynaMaterial.hpp"
#include "settings/UserSettings.hpp"

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mutex.hpp>
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
        Ref<Mutex> mutex;
        Ref<MaterialParser> parser;
        Ref<StandardMaterial3D> _unknown_material;
        Ref<ImageTexture> _unknown_texture;
        UserSettings *user_settings_node;
        Dictionary _textures;
        Dictionary _materials;

        bool use_alpha_transparency;

        const char* _transparency_codes[3];

        void _clear_cache();
        void _on_config_changed();

    protected:
        static void _bind_methods();

        public:
            MaterialManager();
            ~MaterialManager() override;

        String get_material_path(const String &model_name, const String &material_name);
        
        Ref<MaszynaMaterial> load_material(const String &model_path, const String &material_name);
        Ref<StandardMaterial3D> get_material(
            const String &model_path,
            const String &material_path,
            Transparency transparent = Disabled,
            bool is_sky = false,
            const Color &diffuse_color = Color(1.0, 1.0, 1.0)
        );
        Ref<ImageTexture> get_texture(const String &texture_path);
        Ref<ImageTexture> load_texture(const String &model_path, const String &material_name);
        Ref<ImageTexture> load_submodel_texture(const String &model_path, const String &material_name);
    };

} //namespace godot

VARIANT_ENUM_CAST(godot::MaterialManager::Transparency);