#include "MaterialManager.hpp"
#include "MaterialParser.hpp"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include "resources/material/MaszynaMaterial.hpp"
#include "settings/UserSettings.hpp"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>


namespace godot {
    MaterialManager *MaterialManager::singleton = nullptr;

    void MaterialManager::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_material", "model_path", "material_path", "transparent", "is_sky", "diffuse_color"), &MaterialManager::get_material, DEFVAL(Transparency::Disabled), DEFVAL(false), DEFVAL(Color(1,1,1)));
        ClassDB::bind_method(D_METHOD("get_texture", "texture_path"), &MaterialManager::get_texture);
        ClassDB::bind_method(D_METHOD("load_texture", "model_path", "material_name", "global"), &MaterialManager::load_texture, DEFVAL(true));
        ClassDB::bind_method(D_METHOD("load_submodel_texture", "model_path", "material_name"), &MaterialManager::load_submodel_texture);

        ClassDB::bind_method(D_METHOD("_clear_cache"), &MaterialManager::_clear_cache);
        ClassDB::bind_method(D_METHOD("_on_config_changed"), &MaterialManager::_on_config_changed);

        BIND_ENUM_CONSTANT(Disabled);
        BIND_ENUM_CONSTANT(Alpha);
        BIND_ENUM_CONSTANT(AlphaScissor);
    }

    MaterialManager *MaterialManager::get_singleton() {
        return singleton;
    }

    String MaterialManager::get_material_path(const String &model_name, const String &material_name) const {
        if (user_settings_node == nullptr) {
            return "";
        }
        String project_data_dir = user_settings_node->get_maszyna_game_dir();
        PackedStringArray _possible_paths = {
            project_data_dir+"/"+model_name+"/"+material_name+".mat",
            project_data_dir+"/textures/"+model_name+"/"+material_name+".mat",
            project_data_dir+"/"+material_name+".mat",
            project_data_dir+"/"+"textures/"+material_name+".mat",
        };

        for (String _file: _possible_paths) {
            if (FileAccess::file_exists(_file)) {
                return _file;
            }
        }

        return "";
    }

    MaterialManager::MaterialManager() {
        parser = memnew(MaterialParser);
        user_settings_node = nullptr; // Will be set on enter tree when SceneTree is available
        ERR_FAIL_COND(singleton != nullptr);
        singleton = this;
        // Defer loading of fallback resources to when they are actually needed
        // or when the node enters the scene tree. This avoids touching the RenderingServer
        // during doctool/editor headless runs and prevents shutdown errors.
    }

    MaterialManager::~MaterialManager() {
        // Ensure we drop refs before the RenderingServer is torn down.
        _unknown_material.unref();
        _unknown_texture.unref();
        ERR_FAIL_COND(singleton != this);
        singleton = nullptr;
        memdelete(parser);
    }

    void MaterialManager::_notification(const int p_what) {
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                // SceneTree should be available now; acquire UserSettings if not set
                if (user_settings_node == nullptr && get_tree() && get_tree()->get_root()) {
                    user_settings_node = get_tree()->get_root()->get_node<UserSettings>("UserSettings");
                }
                _clear_cache(); // Initial setup
                if (user_settings_node) {
                    // Assuming UserSettings is an autoload singleton
                    user_settings_node->connect("config_changed", callable_mp(this, &MaterialManager::_on_config_changed));
                    user_settings_node->connect("cache_clear_requested", callable_mp(this, &MaterialManager::_clear_cache));
                }
            } break;
            case NOTIFICATION_EXIT_TREE:
            case NOTIFICATION_PREDELETE: {
                // Drop references to GPU resources as early as possible.
                _unknown_material.unref();
                _unknown_texture.unref();
                _textures.clear();
                _materials.clear();
            } break;
            default: ;
        }
    }

    void MaterialManager::_clear_cache() {
        _textures.clear();
        _materials.clear();
        use_alpha_transparency = false; //UserSettings::get_setting("e3d", "use_alpha_transparency", "false");
        if (user_settings_node) {
            Variant v = user_settings_node->call("get_setting", "e3d", "use_alpha_transparency", false);
            if (v.get_type() != Variant::NIL) {
                use_alpha_transparency = static_cast<bool>(v);
            }
        }
    }

    void MaterialManager::_on_config_changed() {
        _clear_cache();
    }

    Ref<MaszynaMaterial> MaterialManager::load_material(const String &model_path, const String &material_name) const {
        return parser->parse(model_path, material_name);
    }

    Ref<StandardMaterial3D> MaterialManager::get_material(
        const String &model_path,
        const String &material_path,
        Transparency transparent,
        const bool is_sky,
        const Color &diffuse_color
    ) {
        const String _code = model_path + String("/") + material_path + ":t=" + _transparency_codes[transparent] + ":s=" + (is_sky ? "1" : "0");

        if (_materials.has(_code)) {
            return _materials[_code];
        }

        Ref<StandardMaterial3D> _m;
        _m.instantiate();

        Ref<MaszynaMaterial> _mmat = load_material(model_path, material_path);

        if (_mmat.is_valid()) {
            if (!_mmat->get_albedo_texture_path().is_empty()) {
                _m->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, load_texture(model_path, _mmat->get_albedo_texture_path()));
            } else {
                // possibly "COLORED" material
                _m->set_albedo(diffuse_color);
            }

            if (const String _normal = _mmat->get_normal_texture_path(); !_normal.is_empty()) {
                if (const Ref<ImageTexture> normal_tex = load_texture(model_path, _normal); normal_tex.is_valid() && normal_tex->get_image().is_valid()) {
                    const Ref<Image> im = normal_tex->get_image();
                    im->decompress();
                    im->normal_map_to_xy();
                    im->compress(Image::COMPRESS_S3TC, Image::COMPRESS_SOURCE_NORMAL);
                    Ref<ImageTexture> new_normal_tex;
                    new_normal_tex.instantiate();
                    new_normal_tex->create_from_image(im);
                    _m->set_texture(BaseMaterial3D::TEXTURE_NORMAL, new_normal_tex);
                    _m->set_feature(StandardMaterial3D::FEATURE_NORMAL_MAPPING, true);
                    _m->set_normal_scale(-5.0);
                }
            }
            _mmat->apply_to_material(*_m);
        }

        _m->set_alpha_scissor_threshold(0.5); // default

        // DETECT ALPHA FROM TEXTURE
        bool texture_alpha = false;
        if (const Ref<Texture2D> _tex_alpha = _m->get_texture(BaseMaterial3D::TEXTURE_ALBEDO); _tex_alpha.is_valid()) {
            if (const Ref<Image> img = _tex_alpha->get_image(); img.is_valid()) {
                texture_alpha = img->detect_alpha() != Image::ALPHA_NONE;
            }
            if(!texture_alpha) {
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

        _materials[_code] = _m;
        return _m;
    }

    Ref<ImageTexture> MaterialManager::get_texture(const String &texture_path) {
        return load_texture("", texture_path);
    }

    Ref<ImageTexture> MaterialManager::load_texture(const String &model_path, const String &material_name, bool global) {
        auto ensure_fallback = [&]() {
            if (!_unknown_texture.is_valid()) {
                if (ResourceLoader *rl = ResourceLoader::get_singleton()) {
                    _unknown_texture = rl->load("res://addons/libmaszyna/materials/missing_texture.png");
                }
            }
        };
        if (user_settings_node == nullptr) {
            ensure_fallback();
            return _unknown_texture;
        }
        String project_data_dir = user_settings_node->call("get_maszyna_game_dir");

        if (project_data_dir.ends_with("\\") || project_data_dir.ends_with("/")) {
            project_data_dir = project_data_dir.substr(0, project_data_dir.length() - 1);
        }

        PackedStringArray possible_paths;
        possible_paths.push_back(project_data_dir.path_join(model_path).path_join(material_name + String(".dds")));
        possible_paths.push_back(project_data_dir.path_join("textures").path_join(model_path).path_join(material_name + String(".dds")));
        possible_paths.push_back(project_data_dir.path_join(material_name + String(".dds")));
        possible_paths.push_back(project_data_dir.path_join("textures").path_join(material_name + String(".dds")));

        String final_path;
        for (int i = 0; i < possible_paths.size(); ++i) {
            if (FileAccess::file_exists(possible_paths[i])) {
                final_path = possible_paths[i];
                break;
            }
        }

        if (final_path.is_empty()) {
            return _unknown_texture;
        }

        if (_textures.has(final_path)) {
            return _textures[final_path];
        }

        const Ref<FileAccess> file = FileAccess::open(final_path, FileAccess::READ);
        if (!file.is_valid()) {
            UtilityFunctions::push_error("Error opening texture file: " + final_path);
            return _unknown_texture;
        }

        const PackedByteArray buffer = file->get_buffer(file->get_length());

        Ref<Image> im;
        im.instantiate();

        if (const Error res = im->load_dds_from_buffer(buffer); res == OK) {
            Ref<ImageTexture> tex;
            tex.instantiate();
            tex->create_from_image(im);
            _textures[final_path] = tex;
            return tex;
        }
        UtilityFunctions::push_error("Error loading texture \"" + final_path + "\" ");
        return _unknown_texture;
    }

    Ref<ImageTexture> MaterialManager::load_submodel_texture(const String &model_path, const String &material_name) {
        return load_texture(model_path, material_name, material_name.contains("/"));
    }
}