#include "MaszynaMaterial.hpp"

#include "macros.hpp"
#include <godot_cpp/classes/base_material3d.hpp>

namespace godot {

    void MaszynaMaterial::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                Variant::INT, "transparency", "transparency", &MaszynaMaterial::set_transparency,
                &MaszynaMaterial::get_transparency, "transparency", PROPERTY_HINT_ENUM, "Disabled,Alpha,AlphaScissor");
        BIND_PROPERTY(
                Variant::STRING, "albedo_texture_path", "albedo_texture_path",
                &MaszynaMaterial::set_albedo_texture_path, &MaszynaMaterial::get_albedo_texture_path, "path");
        BIND_PROPERTY(
                Variant::STRING, "normal_texute_path", "normal_texute_path", &MaszynaMaterial::set_normal_texute_path,
                &MaszynaMaterial::get_normal_texute_path, "path");
        BIND_PROPERTY(
                Variant::STRING, "winter_albedo_texture_path", "winter_albedo_texture_path",
                &MaszynaMaterial::set_winter_albedo_texture_path, &MaszynaMaterial::get_winter_albedo_texture_path,
                "path");
        BIND_PROPERTY(
                Variant::STRING, "winter_normal_texute_path", "winter_normal_texute_path",
                &MaszynaMaterial::set_winter_normal_texute_path, &MaszynaMaterial::get_winter_normal_texute_path,
                "path");
        BIND_PROPERTY(
                Variant::STRING, "autumn_albedo_texture_path", "autumn_albedo_texture_path",
                &MaszynaMaterial::set_autumn_albedo_texture_path, &MaszynaMaterial::get_autumn_albedo_texture_path,
                "path");
        BIND_PROPERTY(
                Variant::STRING, "shader", "shader", &MaszynaMaterial::set_shader, &MaszynaMaterial::get_shader,
                "shader");
        BIND_PROPERTY(
                Variant::INT, "shadow_rank", "shadow_rank", &MaszynaMaterial::set_shadow_rank,
                &MaszynaMaterial::get_shadow_rank, "shadow_rank");

        ClassDB::bind_method(D_METHOD("apply_to_material", "material"), &MaszynaMaterial::apply_to_material);
    }

    String MaszynaMaterial::_parse_texture_path(const String &p_path) {
        PackedStringArray parts = p_path.split(":");
        const String texture = parts.size() > 0 ? parts[0] : p_path;
        for (int i = 0; i < parts.size(); i++) {
            if (parts[i] == "t") {
                transparency = BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR;
                break;
            }
        }
        return texture;
    }

    BaseMaterial3D::Transparency MaszynaMaterial::get_transparency() const {
        return transparency;
    }

    void MaszynaMaterial::set_transparency(const BaseMaterial3D::Transparency p_transparency) {
        transparency = p_transparency;
    }

    String MaszynaMaterial::get_albedo_texture_path() const {
        return albedo_texture_path;
    }

    void MaszynaMaterial::set_albedo_texture_path(const String &p_path) {
        albedo_texture_path = _parse_texture_path(p_path);
    }

    String MaszynaMaterial::get_normal_texute_path() const {
        return normal_texute_path;
    }

    void MaszynaMaterial::set_normal_texute_path(const String &p_path) {
        normal_texute_path = p_path;
    }

    String MaszynaMaterial::get_winter_albedo_texture_path() const {
        return winter_albedo_texture_path;
    }

    void MaszynaMaterial::set_winter_albedo_texture_path(const String &p_path) {
        winter_albedo_texture_path = _parse_texture_path(p_path);
    }

    String MaszynaMaterial::get_winter_normal_texute_path() const {
        return winter_normal_texute_path;
    }

    void MaszynaMaterial::set_winter_normal_texute_path(const String &p_path) {
        winter_normal_texute_path = p_path;
    }

    String MaszynaMaterial::get_autumn_albedo_texture_path() const {
        return autumn_albedo_texture_path;
    }

    void MaszynaMaterial::set_autumn_albedo_texture_path(const String &p_path) {
        autumn_albedo_texture_path = _parse_texture_path(p_path);
    }

    String MaszynaMaterial::get_shader() const {
        return shader;
    }

    void MaszynaMaterial::set_shader(const String &p_shader) {
        shader = p_shader;
    }

    int MaszynaMaterial::get_shadow_rank() const {
        return shadow_rank;
    }

    void MaszynaMaterial::set_shadow_rank(const int p_shadow_rank) {
        shadow_rank = p_shadow_rank;
    }

    void MaszynaMaterial::apply_to_material(const Ref<StandardMaterial3D> &p_material) const {
        if (p_material.is_null()) {
            return;
        }

        p_material->set_transparency(transparency);

        if (transparency == BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR) {
            p_material->set_alpha_scissor_threshold(0.1f);
        }

        if (shader == "shadowlessnormalmap") {
            p_material->set_specular_mode(BaseMaterial3D::SPECULAR_DISABLED);
            p_material->set_flag(BaseMaterial3D::FLAG_DONT_RECEIVE_SHADOWS, true);
        } else if (shader == "sunlessnormalmap") {
            p_material->set_specular_mode(BaseMaterial3D::SPECULAR_DISABLED);
        }
    }

} // namespace godot
