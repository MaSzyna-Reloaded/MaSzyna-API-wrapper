#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

    class MaszynaMaterial : public RefCounted {
            GDCLASS(MaszynaMaterial, RefCounted)

        private:
            BaseMaterial3D::Transparency transparency = BaseMaterial3D::TRANSPARENCY_DISABLED;
            String albedo_texture_path;
            String normal_texute_path;
            String winter_albedo_texture_path;
            String winter_normal_texute_path;
            String autumn_albedo_texture_path;
            String shader;
            int shadow_rank = 0;

            String _parse_texture_path(const String &p_path);

        protected:
            static void _bind_methods();

        public:
            BaseMaterial3D::Transparency get_transparency() const;
            void set_transparency(BaseMaterial3D::Transparency p_transparency);

            String get_albedo_texture_path() const;
            void set_albedo_texture_path(const String &p_path);

            String get_normal_texute_path() const;
            void set_normal_texute_path(const String &p_path);

            String get_winter_albedo_texture_path() const;
            void set_winter_albedo_texture_path(const String &p_path);

            String get_winter_normal_texute_path() const;
            void set_winter_normal_texute_path(const String &p_path);

            String get_autumn_albedo_texture_path() const;
            void set_autumn_albedo_texture_path(const String &p_path);

            String get_shader() const;
            void set_shader(const String &p_shader);

            int get_shadow_rank() const;
            void set_shadow_rank(int p_shadow_rank);

            void apply_to_material(const Ref<StandardMaterial3D> &p_material) const;
    };

} // namespace godot
