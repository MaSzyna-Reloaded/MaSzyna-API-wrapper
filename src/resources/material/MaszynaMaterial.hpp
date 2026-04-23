#pragma once
#include "macros.hpp"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

namespace godot {
   class MaszynaMaterial: public Resource {
       GDCLASS(MaszynaMaterial, Resource)
       public:
           MaszynaMaterial();
           void apply_to_material(StandardMaterial3D *p_material) const;
       protected:
           static void _bind_methods();
       private:
           String _parse_texture_path(const String &p_texture_path);
           String albedo_texture_path;
           String winter_albedo_texture_path;
           String autumn_albedo_texture_path;
           MAKE_MEMBER_GS_NR(BaseMaterial3D::Transparency, transparency, StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
           MAKE_MEMBER_GS(String, normal_texture_path, "")
           MAKE_MEMBER_GS(String, winter_normal_texture_path, "")
           MAKE_MEMBER_GS(String, shader, "")
           MAKE_MEMBER_GS(int, shadow_rank, 0)
       public:
           String get_albedo_texture_path() const;
           void set_albedo_texture_path(const String &p_value);
           String get_winter_albedo_texture_path() const;
           void set_winter_albedo_texture_path(const String &p_value);
           String get_autumn_albedo_texture_path() const;
           void set_autumn_albedo_texture_path(const String &p_value);
   };
} //namespace godot
