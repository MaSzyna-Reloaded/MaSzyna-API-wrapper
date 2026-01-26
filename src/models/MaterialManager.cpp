#include "MaterialManager.hpp"
#include "MaterialParser.hpp"
#include "godot_cpp/classes/config_file.hpp"
#include "resources/material/MaszynaMaterial.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    void MaterialManager::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("get_material_path", "model_name", "material_name"), &MaterialManager::get_material_path);
        ClassDB::bind_method(D_METHOD("load_material", "model_path", "material_name"), &MaterialManager::load_material);
        ClassDB::bind_method(
                D_METHOD("get_material", "model_path", "material_path", "transparent", "is_sky", "diffuse_color"),
                &MaterialManager::get_material, DEFVAL(Transparency::Disabled), DEFVAL(false), DEFVAL(Color(1, 1, 1)));
        ClassDB::bind_method(D_METHOD("get_texture", "texture_path"), &MaterialManager::get_texture);
        ClassDB::bind_method(D_METHOD("load_texture", "model_path", "material_name"), &MaterialManager::load_texture);
        ClassDB::bind_method(
                D_METHOD("load_submodel_texture", "model_path", "material_name"),
                &MaterialManager::load_submodel_texture);
        ClassDB::bind_method(D_METHOD("_clear_cache"), &MaterialManager::_clear_cache);
        ClassDB::bind_method(D_METHOD("_on_config_changed"), &MaterialManager::_on_config_changed);
        BIND_ENUM_CONSTANT(Disabled);
        BIND_ENUM_CONSTANT(Alpha);
        BIND_ENUM_CONSTANT(AlphaScissor);
    }

    MaterialManager::MaterialManager() : game_dir(""), parser(memnew(MaterialParser)), user_settings_node(nullptr) {
        mutex.instantiate();
        _transparency_codes[0] = "0";
        _transparency_codes[1] = "a";
        _transparency_codes[2] = "s";
        UtilityFunctions::print("[MaterialManager] Initializing MaterialManager...");
        _clear_cache();

        // Try to find UserSettings if it's already registered as a singleton
        if (Engine::get_singleton()->has_singleton("UserSettings")) {
            UtilityFunctions::print_verbose("[MaterialManager] Engine has UserSettings singleton");
            user_settings_node = Engine::get_singleton()->get_singleton("UserSettings");
            if (const Variant result = user_settings_node->call("get_maszyna_game_dir");
                result.get_type() == Variant::STRING) {
                game_dir = result;
                UtilityFunctions::print_verbose("[MaterialManager] Project data dir: " + game_dir);
            } else {
                UtilityFunctions::push_warning("[MaterialManager] Cannot access MaSzyna game directory via UserSettings singleton. Falling back to manual reading settings file...");
            }
        } else {
            UtilityFunctions::push_warning("[MaterialManager] Cannot access UserSettings singleton. Falling back to manual reading settings file...");
        }

        if (game_dir == "") {
            Ref<ConfigFile> _config = memnew(ConfigFile);
            _config.instantiate();
            _config->load("user://settings.cfg");
            game_dir = _config->get_value("maszyna", "game_dir");
        }

        if (user_settings_node != nullptr) {
            if (Object *mm_singleton = Engine::get_singleton()->get_singleton("MaterialManager");
                mm_singleton != nullptr) {
                user_settings_node->connect("config_changed", Callable(mm_singleton, "_on_config_changed"));
                user_settings_node->connect("cache_clear_requested", Callable(mm_singleton, "_clear_cache"));
            }
        }
    }

    MaterialManager::~MaterialManager() {
        mutex.unref();
    }

    String MaterialManager::get_material_path(const String &model_name, const String &material_name) {
        if (game_dir.ends_with("\\") || game_dir.ends_with("/")) {
            game_dir = game_dir.substr(0, game_dir.length() - 1);
        }

        PackedStringArray _possible_paths = {
                game_dir + "/" + model_name + "/" + material_name + ".mat",
                game_dir + "/textures/" + model_name + "/" + material_name + ".mat",
                game_dir + "/" + material_name + ".mat",
                game_dir + "/" + "textures/" + material_name + ".mat",

        };

        for (String _file: _possible_paths) {
            if (FileAccess::file_exists(_file)) {
                return _file;
            }
        }

        return "";
    }


    void MaterialManager::_clear_cache() {
        mutex->lock();
        _textures.clear();
        _materials.clear();
        use_alpha_transparency = false;
        if (user_settings_node != nullptr) {
            if (const Variant v = user_settings_node->call("get_setting", "e3d", "use_alpha_transparency", false);
                v.get_type() != Variant::NIL) {
                use_alpha_transparency = static_cast<bool>(v);
            }
        }

        mutex->unlock();
    }


    void MaterialManager::_on_config_changed() {
        _clear_cache();
    }


    Ref<MaszynaMaterial> MaterialManager::load_material(const String &model_path, const String &material_name) {
        if (parser.is_null()) {
            parser.instantiate();
        }

        UtilityFunctions::print_verbose("[MaterialManager] Loading material: " + material_name);
        return parser->parse(this, model_path, material_name);
    }


    Ref<StandardMaterial3D> MaterialManager::get_material(
            const String &model_path, const String &material_path, Transparency transparent, const bool is_sky,
            const Color &diffuse_color) {
        const String _code = model_path + String("/") + material_path + ":t=" + _transparency_codes[transparent] +
                             ":s=" + (is_sky ? "1" : "0");
        mutex->lock();
        if (_materials.has(_code)) {
            Ref<StandardMaterial3D> m = _materials.get(_code, "");
            mutex->unlock();
            return m;
        }

        mutex->unlock();
        Ref<StandardMaterial3D> _m = memnew(StandardMaterial3D);
        UtilityFunctions::print_verbose("[MaterialManager] Creating material: " + material_path);
        const Ref<MaszynaMaterial> _mmat = load_material(model_path, material_path);
        UtilityFunctions::print_verbose("[MaterialManager] Loaded material: " + material_path);

        if (_mmat.is_valid()) {
            UtilityFunctions::print_verbose("[MaterialManager] Material is valid: " + material_path);
            if (const String _albedo = _mmat->get_albedo_texture_path(); !_albedo.is_empty()) {
                UtilityFunctions::print_verbose("[MaterialManager] Albedo is not empty: " + _albedo);
                _m->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, load_texture(model_path, _albedo.split(":").get(0)));
            } else {
                // possibly "COLORED" material
                UtilityFunctions::print_verbose("[MaterialManager] Albedo is empty: " + material_path);
                _m->set_albedo(diffuse_color);
            }


            UtilityFunctions::print_verbose("[MaterialManager] Trying to load normal: " + material_path);
            if (const String _normal = _mmat->get_normal_texture_path(); !_normal.is_empty()) {
                if (const Ref<ImageTexture> normal_tex = load_texture(model_path, _normal.split(":").get(0));
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
            UtilityFunctions::push_warning("[MaterialManager] Material is not valid: " + material_path);
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

        mutex->lock();
        _materials.set(_code, _m);
        mutex->unlock();
        return _m;
    }

    Ref<ImageTexture> MaterialManager::get_texture(const String &texture_path) {
        return load_texture("", texture_path);
    }

    Ref<ImageTexture> MaterialManager::load_texture(const String &model_path, const String &material_name) {
        UtilityFunctions::print_verbose(
                "[MaterialManager] Loading texture for: " + model_path + ", " + material_name + "...");

        if (_unknown_texture.is_null()) {
            _unknown_texture.instantiate(); // Create new ImageTexture
        }

        static bool fallback_loaded = false;
        if (!fallback_loaded) {
            if (ResourceLoader *rl = ResourceLoader::get_singleton()) {
                UtilityFunctions::print_verbose("[MaterialManager] Loading fallback texture");
                _unknown_texture = rl->load("res://addons/libmaszyna/materials/missing_texture.png");
                fallback_loaded = true;
            }
        }

        if (game_dir.ends_with("\\") || game_dir.ends_with("/")) {
            game_dir = game_dir.substr(0, game_dir.length() - 1);
        }


        UtilityFunctions::print_verbose("[MaterialManager] Searching for texture in: " + game_dir);
        const PackedStringArray possible_paths = {
                game_dir + "/" + model_path + "/" + material_name + ".dds",
                game_dir + "/textures/" + model_path + "/" + material_name + ".dds",
                game_dir + "/" + material_name + ".dds",
                game_dir + "/" + "textures/" + material_name + ".dds",
        };

        String final_path;
        for (int i = 0; i < possible_paths.size(); ++i) {
            if (FileAccess::file_exists(possible_paths.get(i))) {
                final_path = possible_paths.get(i);
                UtilityFunctions::print_verbose("[MaterialManager] Found in: " + possible_paths.get(i));
                break;
            }

            if (i == possible_paths.size() - 1) {
                UtilityFunctions::print_verbose("[MaterialManager] Texture ", material_name, " not found");
            }
        }


        if (final_path.is_empty()) {
            UtilityFunctions::print_verbose("[MaterialManager] Using fallback texture");
            return _unknown_texture;
        }

        mutex->lock();
        if (_textures.has(final_path)) {
            Ref<ImageTexture> t = _textures.get(final_path, "");
            mutex->unlock();
            return t;
        }

        mutex->unlock();
        const Ref<FileAccess> file = FileAccess::open(final_path, FileAccess::READ);
        if (!file.is_valid()) {
            UtilityFunctions::push_error("Error opening texture file: " + final_path);
            return _unknown_texture;
        }

        UtilityFunctions::print_verbose("[MaterialManager] Loading texture: " + final_path);
        const PackedByteArray buffer = file->get_buffer(static_cast<int64_t>(file->get_length()));
        const Ref<Image> im = memnew(Image);
        if (const Error res = im->load_dds_from_buffer(buffer); res == OK) {
            Ref<ImageTexture> tex = ImageTexture::create_from_image(im);
            mutex->lock();
            _textures.set(final_path, tex);
            mutex->unlock();
            return tex;
        }

        return _unknown_texture;
    }
    Ref<ImageTexture> MaterialManager::load_submodel_texture(const String &model_path, const String &material_name) {
        return load_texture(model_path, material_name);
    }
} // namespace godot
