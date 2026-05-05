#include "MaterialManager.hpp"

#include "core/UserSettings.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void MaterialManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("clear_cache"), &MaterialManager::clear_cache);
        ClassDB::bind_method(
                D_METHOD("get_material", "model_path", "material_path", "transparent", "is_sky", "diffuse_color"),
                &MaterialManager::get_material, DEFVAL(Transparency::DISABLED), DEFVAL(false),
                DEFVAL(Color(1.0, 1.0, 1.0)));
        ClassDB::bind_method(D_METHOD("get_texture", "texture_path"), &MaterialManager::get_texture);

        BIND_ENUM_CONSTANT(DISABLED);
        BIND_ENUM_CONSTANT(ALPHA);
        BIND_ENUM_CONSTANT(ALPHA_SCISSOR);
    }

    MaterialManager::MaterialManager() {
        materials_cache = ResourceCache::create("materials");
        ResourceLoader *resource_loader = ResourceLoader::get_singleton();
        if (resource_loader != nullptr) {
            unknown_texture = resource_loader->load("res://addons/libmaszyna/materials/missing_texture.png");
        }
        clear_cache();
    }

    String MaterialManager::_get_transparency_code(const Transparency p_transparency) const {
        switch (p_transparency) {
            case Transparency::ALPHA:
                return "a";
            case Transparency::ALPHA_SCISSOR:
                return "s";
            case Transparency::DISABLED:
            default:
                return "0";
        }
    }

    void MaterialManager::clear_cache() {
        if (materials_cache.is_valid()) {
            materials_cache->clear();
        }

        UserSettings *user_settings = UserSettings::get_instance();
        if (user_settings != nullptr) {
            use_alpha_transparency =
                    static_cast<bool>(user_settings->get_setting("e3d", "use_alpha_transparency", false));
        }
    }

    Ref<MaszynaMaterial>
    MaterialManager::load_material(const String &p_model_path, const String &p_material_name) const {
        Ref<MaterialParser> parser;
        parser.instantiate();
        return parser->parse(p_model_path, p_material_name);
    }

    Ref<StandardMaterial3D> MaterialManager::get_material(
            const String &p_model_path, const String &p_material_path, const Transparency p_transparent,
            const bool p_is_sky, const Color &p_diffuse_color) const {
        const String cache_code = p_model_path.path_join(vformat(
                "%s_t%s_s%s_%02x%02x%02x.res", p_material_path, _get_transparency_code(p_transparent),
                p_is_sky ? "1" : "0", p_diffuse_color.get_r8(), p_diffuse_color.get_g8(), p_diffuse_color.get_b8()));

        Ref<StandardMaterial3D> output = materials_cache->get(cache_code);
        if (output.is_valid()) {
            return output;
        }

        Ref<StandardMaterial3D> material;
        material.instantiate();

        Ref<MaszynaMaterial> parsed_material = load_material(p_model_path, p_material_path);

        if (parsed_material.is_valid() && !parsed_material->get_albedo_texture_path().is_empty()) {
            material->set_texture(
                    BaseMaterial3D::TEXTURE_ALBEDO,
                    load_texture(p_model_path, parsed_material->get_albedo_texture_path()));
        } else {
            material->set_albedo(p_diffuse_color);
        }

        if (parsed_material.is_valid() && !parsed_material->get_normal_texute_path().is_empty()) {
            material->set_texture(
                    BaseMaterial3D::TEXTURE_NORMAL,
                    load_texture(p_model_path, parsed_material->get_normal_texute_path(), true));
            material->set_feature(BaseMaterial3D::FEATURE_NORMAL_MAPPING, true);
            material->set_normal_scale(-5.0f);
        }

        if (parsed_material.is_valid()) {
            parsed_material->apply_to_material(material);
        }

        material->set_alpha_scissor_threshold(0.5f);

        Transparency resolved_transparency = p_transparent;
        bool texture_alpha = false;
        Ref<Texture2D> albedo_texture = material->get_texture(BaseMaterial3D::TEXTURE_ALBEDO);
        if (albedo_texture.is_valid()) {
            Ref<Image> image = albedo_texture->get_image();
            if (image.is_valid()) {
                texture_alpha = image->detect_alpha() != Image::ALPHA_NONE;
            }
        }

        if (texture_alpha) {
            if (use_alpha_transparency) {
                resolved_transparency = Transparency::ALPHA;
            } else {
                resolved_transparency = Transparency::ALPHA_SCISSOR;
                material->set_alpha_scissor_threshold(0.8f);
            }
        }

        if (resolved_transparency == Transparency::ALPHA_SCISSOR) {
            material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
        } else if (resolved_transparency == Transparency::ALPHA) {
            material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA_DEPTH_PRE_PASS);
        }

        if (p_is_sky) {
            material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
            material->set_albedo(Color(0.9, 0.9, 0.9, 0.8));
            material->set_flag(BaseMaterial3D::FLAG_DONT_RECEIVE_SHADOWS, true);
            material->set_flag(BaseMaterial3D::FLAG_DISABLE_AMBIENT_LIGHT, true);
        }

        materials_cache->set(cache_code, material);
        return materials_cache->get(cache_code);
    }

    Ref<Texture2D> MaterialManager::get_texture(const String &p_texture_path) const {
        return load_texture("", p_texture_path);
    }

    Ref<Texture2D> MaterialManager::load_texture(
            const String &p_model_path, const String &p_material_name, const bool p_normal) const {
        static_cast<void>(p_normal);
        UserSettings *user_settings = UserSettings::get_instance();
        if (user_settings == nullptr) {
            return unknown_texture;
        }

        String project_data_dir = user_settings->get_maszyna_game_dir();
        if (project_data_dir.ends_with("\\")) {
            project_data_dir = project_data_dir.trim_suffix("\\");
        }
        if (project_data_dir.ends_with("/")) {
            project_data_dir = project_data_dir.trim_suffix("/");
        }

        const String texture_filename = p_material_name + String(".dds");
        PackedStringArray possible_paths;
        possible_paths.append(p_model_path.path_join(texture_filename));
        possible_paths.append(String("textures").path_join(p_model_path.path_join(texture_filename)));
        possible_paths.append(texture_filename);
        possible_paths.append(String("textures").path_join(texture_filename));

        String final_path;
        for (int i = 0; i < possible_paths.size(); i++) {
            if (FileAccess::file_exists(project_data_dir.path_join(possible_paths[i]))) {
                final_path = possible_paths[i];
                break;
            }
        }

        if (final_path.is_empty()) {
            return unknown_texture;
        }

        ResourceLoader *resource_loader = ResourceLoader::get_singleton();
        if (resource_loader == nullptr) {
            return unknown_texture;
        }

        Ref<Texture2D> texture = resource_loader->load(project_data_dir.path_join(final_path));
        if (texture.is_null()) {
            return unknown_texture;
        }

        return texture;
    }

} // namespace godot
