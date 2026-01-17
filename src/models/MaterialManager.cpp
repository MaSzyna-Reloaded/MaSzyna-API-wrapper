#include "MaterialManager.hpp"
#include "MaterialParser.hpp"
#include "resources/material/MaszynaMaterial.hpp"
#include "settings/UserSettings.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>


namespace godot {
    MaterialParser *MaterialManager::parser = nullptr;
    Ref<StandardMaterial3D> *MaterialManager::_unknown_material = nullptr;
    Ref<ImageTexture> *MaterialManager::_unknown_texture = nullptr;
    UserSettings *MaterialManager::user_settings_node = nullptr;
    Dictionary *MaterialManager::_textures = nullptr;
    Dictionary *MaterialManager::_materials = nullptr;
    bool MaterialManager::use_alpha_transparency = false;
    const char *MaterialManager::_transparency_codes[3] = {"0", "a", "s"};

    void MaterialManager::_bind_methods() {
        ClassDB::bind_static_method(
                "MaterialManager",
                D_METHOD("get_material", "model_path", "material_path", "transparent", "is_sky", "diffuse_color"),
                &MaterialManager::get_material, DEFVAL(Transparency::Disabled), DEFVAL(false), DEFVAL(Color(1, 1, 1)));
        ClassDB::bind_static_method(
                "MaterialManager", D_METHOD("get_texture", "texture_path"), &MaterialManager::get_texture);
        ClassDB::bind_static_method(
                "MaterialManager", D_METHOD("load_texture", "model_path", "material_name", "global"),
                &MaterialManager::load_texture, DEFVAL(true));
        ClassDB::bind_static_method(
                "MaterialManager", D_METHOD("load_submodel_texture", "model_path", "material_name"),
                &MaterialManager::load_submodel_texture);

        ClassDB::bind_static_method("MaterialManager", D_METHOD("_clear_cache"), &MaterialManager::_clear_cache);
        ClassDB::bind_static_method(
                "MaterialManager", D_METHOD("_on_config_changed"), &MaterialManager::_on_config_changed);

        BIND_ENUM_CONSTANT(Disabled);
        BIND_ENUM_CONSTANT(Alpha);
        BIND_ENUM_CONSTANT(AlphaScissor);
    }

    void MaterialManager::init() {
        UtilityFunctions::print("[MaterialManager] Initializing MaterialManager...");
        if (parser == nullptr) {
            parser = memnew(MaterialParser);
        }
        if (_textures == nullptr) {
            _textures = memnew(Dictionary);
        }
        if (_materials == nullptr) {
            _materials = memnew(Dictionary);
        }
        if (_unknown_texture == nullptr) {
            _unknown_texture = memnew(Ref<ImageTexture>);
        }
        if (_unknown_material == nullptr) {
            _unknown_material = memnew(Ref<StandardMaterial3D>);
        }
        _clear_cache();

        // Try to find UserSettings if it's already registered as a singleton
        if (Engine::get_singleton()->has_singleton("UserSettings")) {
            user_settings_node = Object::cast_to<UserSettings>(Engine::get_singleton()->get_singleton("UserSettings"));
        }

        if (user_settings_node != nullptr) {
            Object *mm_singleton = Engine::get_singleton()->get_singleton("MaterialManager");
            user_settings_node->connect(
                    "config_changed",
                    Callable(mm_singleton, "_on_config_changed"));
            user_settings_node->connect(
                    "cache_clear_requested",
                    mm_singleton != nullptr ? Callable(mm_singleton, "_clear_cache") : Callable());
        }
    }

    void MaterialManager::cleanup() {
        if (_unknown_material != nullptr) {
            _unknown_material->unref();
            memdelete(_unknown_material);
            _unknown_material = nullptr;
        }

        if (_unknown_texture != nullptr) {
            _unknown_texture->unref();
            memdelete(_unknown_texture);
            _unknown_texture = nullptr;
        }

        if (_textures != nullptr) {
            memdelete(_textures);
            _textures = nullptr;
        }
        if (_materials != nullptr) {
            memdelete(_materials);
            _materials = nullptr;
        }
        if (parser != nullptr) {
            memdelete(parser);
            parser = nullptr;
        }
    }

    String MaterialManager::get_material_path(const String &model_name, const String &material_name) {
        if (user_settings_node == nullptr) {
            // Fallback if not initialized or not found
            if (Engine::get_singleton()->has_singleton("UserSettings")) {
                user_settings_node =
                        Object::cast_to<UserSettings>(Engine::get_singleton()->get_singleton("UserSettings"));
            }
            if (user_settings_node == nullptr) {
                return "";
            }
        }
        String project_data_dir = "/home/DoS/Games/SteamLibrary/steamapps/common/MaSzyna/";
        PackedStringArray _possible_paths = {
                project_data_dir + "/" + model_name + "/" + material_name + ".mat",
                project_data_dir + "/textures/" + model_name + "/" + material_name + ".mat",
                project_data_dir + "/" + material_name + ".mat",
                project_data_dir + "/" + "textures/" + material_name + ".mat",
        };

        for (auto _file : _possible_paths) {
            if (FileAccess::file_exists(_file)) {
                return _file;
            }
        }

        return "";
    }

    void MaterialManager::_clear_cache() {
        if (_textures != nullptr) {
            _textures->clear();
        }
        if (_materials != nullptr) {
            _materials->clear();
        }
        use_alpha_transparency = false;
        if (user_settings_node != nullptr) {
            if (Variant v = user_settings_node->call("get_setting", "e3d", "use_alpha_transparency", false);
                v.get_type() != Variant::NIL) {
                use_alpha_transparency = static_cast<bool>(v);
            }
        }
    }

    void MaterialManager::_on_config_changed() {
        _clear_cache();
    }

    Ref<MaszynaMaterial> MaterialManager::load_material(const String &model_path, const String &material_name) {
        if (parser == nullptr) {
            parser = memnew(MaterialParser);
        }
        UtilityFunctions::print("[MaterialManager]", " Loading material: " + material_name);
        return parser->parse(model_path, material_name);
    }

    Ref<StandardMaterial3D> MaterialManager::get_material(
            const String &model_path, const String &material_path, Transparency transparent, const bool is_sky,
            const Color &diffuse_color) {
        const String _code = model_path + String("/") + material_path + ":t=" + _transparency_codes[transparent] +
                             ":s=" + (is_sky ? "1" : "0");

        if (_materials != nullptr && _materials->has(_code)) {
            return _materials->get(_code, "");
        }

        Ref<StandardMaterial3D> _m = memnew(StandardMaterial3D);
        UtilityFunctions::print("MaterialManager", "Creating material: " + material_path);
        const Ref<MaszynaMaterial> _mmat = load_material(model_path, material_path);
        UtilityFunctions::print("MaterialManager", "Loaded material: " + material_path);
        if (_mmat.is_valid()) {
            UtilityFunctions::print("MaterialManager", "Material is valid: " + material_path);

            UtilityFunctions::print("[MaterialManager] ", "Trying to load normal: " + material_path);
            if (!_mmat->get_albedo_texture_path().is_empty()) {

                UtilityFunctions::print(
                        "[MaterialManager] ", "Albedo is NOT empty: " + _mmat->get_albedo_texture_path());
                _m->set_texture(
                        BaseMaterial3D::TEXTURE_ALBEDO, load_texture(model_path, _mmat->get_albedo_texture_path()));
            } else {
                // possibly "COLORED" material
                UtilityFunctions::print("[MaterialManager] ", "Albedo is empty: " + material_path);
                _m->set_albedo(diffuse_color);
            }


            UtilityFunctions::print("[MaterialManager] ", "Trying to load normal: " + material_path);
            if (const String _normal = _mmat->get_normal_texture_path(); !_normal.is_empty()) {
                if (const Ref<ImageTexture> normal_tex = load_texture(model_path, _normal);
                    normal_tex.is_valid() && normal_tex->get_image().is_valid()) {
                    const Ref<Image> im = normal_tex->get_image();
                    im->decompress();
                    im->normal_map_to_xy();
                    im->compress(Image::COMPRESS_S3TC, Image::COMPRESS_SOURCE_NORMAL);
                    const Ref<ImageTexture> new_normal_tex = ImageTexture::create_from_image(im);
                    _m->set_texture(BaseMaterial3D::TEXTURE_NORMAL, new_normal_tex);
                    _m->set_feature(StandardMaterial3D::FEATURE_NORMAL_MAPPING, true);
                    _m->set_normal_scale(-5.0);
                }
            }
            _mmat->apply_to_material(*_m);
        } else {

            UtilityFunctions::push_warning("MaterialManager", "Material is NOT valid: " + material_path);
        }

        _m->set_alpha_scissor_threshold(0.5); // default

        // DETECT ALPHA FROM TEXTURE
        bool texture_alpha = false;
        if (const Ref<Texture2D> _tex_alpha = _m->get_texture(BaseMaterial3D::TEXTURE_ALBEDO); _tex_alpha.is_valid()) {
            if (const Ref<Image> img = _tex_alpha->get_image(); img.is_valid()) {
                texture_alpha = img->detect_alpha() != Image::ALPHA_NONE;
            }
            if (!texture_alpha) {
                texture_alpha = _tex_alpha->has_alpha();
            }
        }

        if (texture_alpha) {
            if (use_alpha_transparency) {
                transparent = Transparency::Alpha;
            } else {
                transparent = Transparency::AlphaScissor;
                _m->set_alpha_scissor_threshold(0.80); // Who knows...
            }
        }

        // force transparency and shading
        switch (transparent) {
            case Transparency::AlphaScissor:
                _m->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
                break;
            case Transparency::Alpha:
                _m->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_DEPTH_PRE_PASS);
                break;
            case Transparency::Disabled:
                // default
                break;
        }

        if (is_sky) {
            _m->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
            _m->set_albedo(Color(0.9, 0.9, 0.9, 0.8));
            _m->set_flag(StandardMaterial3D::FLAG_DONT_RECEIVE_SHADOWS, true);
            _m->set_flag(StandardMaterial3D::FLAG_DISABLE_AMBIENT_LIGHT, true);
        }

        if (_materials != nullptr) {
            _materials->set(_code, _m);
        }
        return _m;
    }

    Ref<ImageTexture> MaterialManager::get_texture(const String &texture_path) {
        return load_texture("", texture_path);
    }

    Ref<ImageTexture>
    MaterialManager::load_texture(const String &model_path, const String &material_name, bool global) {

        UtilityFunctions::print("[MaterialManager] ", "Loading texture for: " + model_path + ", " + material_name + "...");

        if (_unknown_texture == nullptr) {
            _unknown_texture = memnew(Ref<ImageTexture>);
        }

        if (_unknown_texture->is_null()) {
            if (ResourceLoader *rl = ResourceLoader::get_singleton()) {
                UtilityFunctions::print("[MaterialManager] ", "Loading fallback texture");
                *_unknown_texture = rl->load("res://addons/libmaszyna/materials/missing_texture.png");
            }
        }
        
        String project_data_dir;
        if (user_settings_node != nullptr) {
            project_data_dir = user_settings_node->get_maszyna_game_dir();
        }
        
        if (project_data_dir.is_empty()) {
             project_data_dir = "/home/DoS/Games/SteamLibrary/steamapps/common/MaSzyna/";
        }

        if (project_data_dir.ends_with("\\") || project_data_dir.ends_with("/")) {
            project_data_dir = project_data_dir.substr(0, project_data_dir.length() - 1);
        }

        UtilityFunctions::print("[MaterialManager] ", "Searching for texture in: " + project_data_dir);
        PackedStringArray possible_paths;
        possible_paths.push_back(project_data_dir.path_join(model_path).path_join(material_name + String(".dds")));
        possible_paths.push_back(project_data_dir.path_join(model_path).path_join(material_name + String(".dds")));
        possible_paths.push_back(
                project_data_dir.path_join(model_path).path_join(material_name + String(".dds")));
        possible_paths.push_back(project_data_dir.path_join(material_name + String(".dds")));
        possible_paths.push_back(project_data_dir.path_join("textures").path_join(material_name + String(".dds")));

        String final_path;
        for (int i = 0; i < possible_paths.size(); ++i) {
            UtilityFunctions::print("[MaterialManager] ", "Checking path: " + possible_paths.get(i));
            if (FileAccess::file_exists(possible_paths.get(i))) {
                final_path = possible_paths.get(i);
                break;
            }
        }

        if (final_path.is_empty()) {
            UtilityFunctions::print("[MaterialManager] ", "Using fallback texture");
            return (_unknown_texture != nullptr) ? *_unknown_texture : Ref<ImageTexture>();
        }

        if (_textures != nullptr && _textures->has(final_path)) {
            return _textures->get(final_path, "");
        }

        const Ref<FileAccess> file = FileAccess::open(final_path, FileAccess::READ);
        if (!file.is_valid()) {
            UtilityFunctions::push_error("Error opening texture file: " + final_path);
            return (_unknown_texture != nullptr) ? *_unknown_texture : Ref<ImageTexture>();
        }

        UtilityFunctions::print("MaterialManager", "Loading texture: " + final_path);
        const PackedByteArray buffer = file->get_buffer(static_cast<int64_t>(file->get_length()));

        const Ref<Image> im = memnew(Image);

        if (const Error res = im->load_dds_from_buffer(buffer); res == OK) {
            Ref<ImageTexture> tex = ImageTexture::create_from_image(im);
            if (_textures != nullptr) {
                _textures->set(final_path, tex);
            }
            return tex;
        }
        UtilityFunctions::push_error("Error loading texture \"" + final_path + "\" ");
        return (_unknown_texture != nullptr) ? *_unknown_texture : Ref<ImageTexture>();
    }

    Ref<ImageTexture> MaterialManager::load_submodel_texture(const String &model_path, const String &material_name) {
        return load_texture(model_path, material_name, material_name.contains("/"));
    }
} // namespace godot
