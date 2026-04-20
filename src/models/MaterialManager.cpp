#include "MaterialManager.hpp"
#include "MaterialParser.hpp"
#include "core/ResourceCache.hpp"
#include "godot_cpp/classes/config_file.hpp"
#include "resources/material/MaszynaMaterial.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/os.hpp>
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
                &MaterialManager::get_material, DEFVAL(Transparency::DISABLED), DEFVAL(false), DEFVAL(Color(1, 1, 1)));
        ClassDB::bind_method(D_METHOD("get_texture", "texture_path"), &MaterialManager::get_texture);
        ClassDB::bind_method(D_METHOD("load_texture", "model_path", "material_name"), &MaterialManager::load_texture);

        ClassDB::bind_static_method(
                get_class_static(), D_METHOD("generate_heightmap_from_albedo", "albedo"),
                &MaterialManager::generate_heightmap_from_albedo);
        ClassDB::bind_static_method(
                get_class_static(), D_METHOD("generate_normal_from_albedo", "albedo", "strength"),
                &MaterialManager::generate_normal_from_albedo, DEFVAL(1.0f));
        ClassDB::bind_static_method(
                get_class_static(), D_METHOD("generate_metallic_from_albedo", "albedo", "threshold"),
                &MaterialManager::generate_metallic_from_albedo, DEFVAL(0.5f));

        BIND_ENUM_CONSTANT(DISABLED);
        BIND_ENUM_CONSTANT(ALPHA);
        BIND_ENUM_CONSTANT(ALPHA_SCISSOR);
    }

    MaterialManager::~MaterialManager() {
        _materials.clear();
        _unknown_material.unref();
        _unknown_texture.unref();
        mutex.unref();
        parser.unref();
    }

    MaterialManager::MaterialManager() {
        mutex.instantiate();
        parser.instantiate();
        UtilityFunctions::print("[MaterialManager] Initializing MaterialManager...");
        _clear_cache();
    }

    String MaterialManager::get_material_path(const String &p_model_name, const String &p_material_name) {
        if (game_dir.ends_with("\\") || game_dir.ends_with("/")) {
            game_dir = game_dir.substr(0, game_dir.length() - 1);
        }

        PackedStringArray possible_paths = {
                game_dir + "/" + p_model_name + "/" + p_material_name + ".mat",
                game_dir + "/textures/" + p_model_name + "/" + p_material_name + ".mat",
                game_dir + "/" + p_material_name + ".mat",
                game_dir + "/" + "textures/" + p_material_name + ".mat",
        };

        for (String file: possible_paths) {
            UtilityFunctions::print_verbose("[MaterialManager] Trying to load material: " + file);
            if (FileAccess::file_exists(file)) {
                return file;
            }
        }

        return "";
    }


    void MaterialManager::_clear_cache() {
        mutex->lock();
        _materials.clear();
        ResourceCache::clear(ResourceCache::RESOURCE_CACHE_DIR_TEXTURES);

        Ref<ConfigFile> config;
        config.instantiate();
        if (config->load("user://settings.cfg") == OK) {
            use_alpha_transparency = config->get_value("e3d", "use_alpha_transparency", false);
            auto_generate_normal = config->get_value("e3d", "auto_generate_normal", false);
            auto_generate_metallic = config->get_value("e3d", "auto_generate_metallic", false);
            auto_generate_height = config->get_value("e3d", "auto_generate_height", false);
            if (OS::get_singleton()->has_feature("release") && !OS::get_singleton()->has_feature("editor")) {
                game_dir = ".";
            } else {
                game_dir = config->get_value("maszyna", "game_dir", ".");
            }
        } else {
            use_alpha_transparency = false;
            auto_generate_normal = false;
            auto_generate_metallic = false;
            auto_generate_height = false;
            game_dir = ".";
        }
        config.unref();
        UtilityFunctions::print_verbose("[MaterialManager] Config loaded. Project data dir: " + game_dir);
        mutex->unlock();
    }

    Ref<MaszynaMaterial> MaterialManager::load_material(const String &p_model_path, const String &p_material_name) {
        if (parser.is_null()) {
            parser.instantiate();
        }

        return parser->parse(get_material_path(p_model_path, p_material_name), p_material_name);
    }


    Ref<StandardMaterial3D> MaterialManager::get_material(
            const String &p_model_path, const String &p_material_path, Transparency p_transparent, const bool p_is_sky,
            const Color &p_diffuse_color) {

        const char *t_code = "0";
        if (p_transparent >= 0 && p_transparent < 3) {
            t_code = _transparency_codes[p_transparent];
        }

        const String code = p_model_path + String("/") + p_material_path + ":t=" + t_code + ":s=" + (p_is_sky ? "1" : "0");
        mutex->lock();
        if (_materials.has(code)) {
            Ref<StandardMaterial3D> m = _materials.get(code, "");
            mutex->unlock();
            return m;
        }

        mutex->unlock();
        Ref<StandardMaterial3D> m = memnew(StandardMaterial3D);
        UtilityFunctions::print_verbose("[MaterialManager] Creating material: " + p_material_path);
        const Ref<MaszynaMaterial> mmat = load_material(p_model_path, p_material_path);
        UtilityFunctions::print_verbose("[MaterialManager] Loaded material: " + p_material_path);

        if (mmat.is_valid()) {
            UtilityFunctions::print_verbose("[MaterialManager] Material is valid: " + p_material_path);
            if (const String albedo = mmat->get_albedo_texture_path(); !albedo.is_empty()) {
                UtilityFunctions::print_verbose("[MaterialManager] Albedo is not empty: " + albedo);
                const Ref<ImageTexture> albedo_tex = load_texture(p_model_path, albedo.split(":").get(0));
                m->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, albedo_tex);

                if (albedo_tex.is_valid()) {
                    const Ref<Image> albedo_img = albedo_tex->get_image();
                    if (albedo_img.is_valid()) {
                        if (auto_generate_metallic) {
                            const Ref<Image> metallic_img = generate_metallic_from_albedo(albedo_img);
                            if (metallic_img.is_valid()) {
                                UtilityFunctions::print_verbose(
                                        "[MaterialManager] Auto-generated and applied metallic texture for: " +
                                        p_material_path);
                                m->set_texture(
                                        BaseMaterial3D::TEXTURE_METALLIC,
                                        ImageTexture::create_from_image(metallic_img));
                                m->set_metallic(1.0);
                            }
                        }

                        // Auto-generate Normal if not explicitly provided
                        if (auto_generate_normal && mmat->get_normal_texture_path().is_empty()) {
                            const Ref<Image> normal_img = generate_normal_from_albedo(albedo_img);
                            if (normal_img.is_valid()) {
                                UtilityFunctions::print_verbose(
                                        "[MaterialManager] Auto-generated and applied normal texture for: " +
                                        p_material_path);
                                m->set_texture(
                                        BaseMaterial3D::TEXTURE_NORMAL, ImageTexture::create_from_image(normal_img));
                                m->set_feature(StandardMaterial3D::FEATURE_NORMAL_MAPPING, true);
                            }
                        }

                        // Auto-generate Height
                        if (auto_generate_height) {
                            const Ref<Image> height_img = generate_heightmap_from_albedo(albedo_img);
                            if (height_img.is_valid()) {
                                UtilityFunctions::print_verbose(
                                        "[MaterialManager] Auto-generated and applied height texture for: " +
                                        p_material_path);
                                m->set_texture(
                                        BaseMaterial3D::TEXTURE_HEIGHTMAP, ImageTexture::create_from_image(height_img));
                                m->set_feature(StandardMaterial3D::FEATURE_HEIGHT_MAPPING, true);
                                m->set_heightmap_scale(0.01);
                            }
                        }
                    }
                }
            } else {
                // possibly "COLORED" material
                UtilityFunctions::print_verbose("[MaterialManager] Albedo is empty: " + p_material_path);
                m->set_albedo(p_diffuse_color);
            }


            UtilityFunctions::print_verbose("[MaterialManager] Trying to load normal: " + p_material_path);
            if (const String normal = mmat->get_normal_texture_path(); !normal.is_empty()) {
                if (const Ref<ImageTexture> normal_tex = load_texture(p_model_path, normal.split(":").get(0));
                    normal_tex.is_valid() && normal_tex->get_image().is_valid()) {
                    const Ref<Image> im = normal_tex->get_image();
                    im->decompress();
                    im->normal_map_to_xy();
                    im->compress(Image::COMPRESS_S3TC, Image::COMPRESS_SOURCE_NORMAL);
                    const Ref<ImageTexture> new_normal_tex = ImageTexture::create_from_image(im);
                    m->set_texture(BaseMaterial3D::TEXTURE_NORMAL, new_normal_tex);
                    m->set_feature(StandardMaterial3D::FEATURE_NORMAL_MAPPING, true);
                    m->set_normal_scale(-5.0);
                }
            }

            mmat->apply_to_material(m.ptr());
        } else {
            UtilityFunctions::push_warning("[MaterialManager] Material is not valid: " + p_material_path);
        }

        m->set_alpha_scissor_threshold(0.5); // default
        // DETECT ALPHA FROM TEXTURE
        bool texture_alpha = false;
        if (const Ref<Texture2D> tex_alpha = m->get_texture(BaseMaterial3D::TEXTURE_ALBEDO); tex_alpha.is_valid()) {
            if (const Ref<Image> img = tex_alpha->get_image(); img.is_valid()) {
                texture_alpha = img->detect_alpha() != Image::ALPHA_NONE;
            }

            if (!texture_alpha) {
                texture_alpha = tex_alpha->has_alpha();
            }
        }

        if (texture_alpha) {
            if (use_alpha_transparency) {
                p_transparent = Transparency::ALPHA;
            } else {
                p_transparent = Transparency::ALPHA_SCISSOR;
                m->set_alpha_scissor_threshold(0.80); // Who knows...
            }
        }

        // force transparency and shading
        switch (p_transparent) {
            case Transparency::ALPHA_SCISSOR:
                m->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
                break;
            case Transparency::ALPHA:
                m->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA_DEPTH_PRE_PASS);
                break;
            case Transparency::DISABLED:
                // default
                break;
        }

        if (p_is_sky) {
            m->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
            m->set_albedo(Color(0.9, 0.9, 0.9, 0.8));
            m->set_flag(StandardMaterial3D::FLAG_DONT_RECEIVE_SHADOWS, true);
            m->set_flag(StandardMaterial3D::FLAG_DISABLE_AMBIENT_LIGHT, true);
        }

        mutex->lock();
        _materials.set(code, m);
        mutex->unlock();
        return m;
    }

    Ref<ImageTexture> MaterialManager::get_texture(const String &p_texture_path) {
        return load_texture("", p_texture_path);
    }

    Ref<ImageTexture> MaterialManager::load_texture(const String &p_model_path, const String &p_material_name) {
        UtilityFunctions::print_verbose(
                "[MaterialManager] Loading texture for: " + p_model_path + ", " + p_material_name + "...");

        if (_unknown_texture.is_null()) {
            _unknown_texture.instantiate(); // Create a new ImageTexture
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
                game_dir + "/" + p_model_path + "/" + p_material_name + ".dds",
                game_dir + "/textures/" + p_model_path + "/" + p_material_name + ".dds",
                game_dir + "/" + p_material_name + ".dds",
                game_dir + "/" + "textures/" + p_material_name + ".dds",
        };

        String final_path;
        for (int i = 0; i < possible_paths.size(); ++i) {
            if (FileAccess::file_exists(possible_paths.get(i))) {
                final_path = possible_paths.get(i);
                UtilityFunctions::print_verbose("[MaterialManager] Found in: " + possible_paths.get(i));
                break;
            }

            if (i == possible_paths.size() - 1) {
                UtilityFunctions::print_verbose("[MaterialManager] Texture ", p_material_name, " not found");
            }
        }


        if (final_path.is_empty()) {
            UtilityFunctions::print_verbose("[MaterialManager] Using fallback texture");
            return _unknown_texture;
        }

        Ref<Resource> cached_res = ResourceCache::get(final_path, ResourceCache::RESOURCE_CACHE_DIR_TEXTURES);
        if (cached_res.is_valid()) {
            return cached_res;
        }

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
            ResourceCache::set(final_path, tex, ResourceCache::RESOURCE_CACHE_DIR_TEXTURES);
            return tex;
        }


        return _unknown_texture;
    }

    Ref<Image> MaterialManager::generate_heightmap_from_albedo(const Ref<Image> &p_albedo) {
        if (p_albedo.is_null()) {
            return nullptr;
        }
        Ref<Image> height = p_albedo->duplicate();
        if (height->is_compressed()) {
            height->decompress();
        }
        height->convert(Image::FORMAT_L8);
        height->adjust_bcs(1.0, 0.5, 1.0); // Reduce contrast to make it less "weird"
        return height;
    }

    Ref<Image> MaterialManager::generate_normal_from_albedo(const Ref<Image> &p_albedo, const float p_strength) {
        Ref<Image> height = generate_heightmap_from_albedo(p_albedo);
        if (height.is_null()) {
            return nullptr;
        }
        height->bump_map_to_normal_map(p_strength);
        return height;
    }

    Ref<Image> MaterialManager::generate_metallic_from_albedo(const Ref<Image> &p_albedo, float p_threshold) {
        if (p_albedo.is_null()) {
            return nullptr;
        }

        Ref<Image> meta = p_albedo->duplicate();
        if (meta->is_compressed()) {
            meta->decompress();
        }
        meta->convert(Image::FORMAT_L8);

        for (int y = 0; y < meta->get_height(); y++) {
            for (int x = 0; x < meta->get_width(); x++) {
                const Color c = meta->get_pixel(x, y);
                if (c.r > p_threshold) {
                    meta->set_pixel(x, y, Color(1, 1, 1));
                } else {
                    meta->set_pixel(x, y, Color(0, 0, 0));
                }
            }
        }
        return meta;
    }
} // namespace godot
