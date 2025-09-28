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
           String _parse_texture_path(const String &texturePath);
           MAKE_MEMBER_OBSERVABLE_GS(String, albedo_texture_path)
           MAKE_MEMBER_GS(BaseMaterial3D::Transparency, transparency, StandardMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);
           MAKE_MEMBER_GS(String, normal_texture_path, "")
           MAKE_MEMBER_OBSERVABLE_GS(String, winter_albedo_texture_path)
           MAKE_MEMBER_GS(String, winter_normal_texture_path, "")
           MAKE_MEMBER_OBSERVABLE_GS(String, autumn_albedo_texture_path)
           MAKE_MEMBER_GS(String, shader, "")
           MAKE_MEMBER_GS(int, shadow_rank, 0)
   };
} //namespace godot