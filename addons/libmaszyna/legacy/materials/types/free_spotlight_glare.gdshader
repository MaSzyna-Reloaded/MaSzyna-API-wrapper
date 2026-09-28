// The glare of a free spotlight at night, in fog or in rain: a 2.5 m billboard turned towards the
// camera around the vertical, added on top without a depth test (opengl33renderer.cpp:4383-4435,
// billboard.frag). The original draws it only if its point passed the depth test (a conditional
// render on the point's occlusion query); here the scene's depth at the light's centre decides.

shader_type spatial;
render_mode unshaded, blend_add, depth_draw_never, depth_test_disabled, cull_disabled, shadows_disabled, fog_disabled;

// The original's Global.fLuminance and Global.Overcast; set by MaszynaEnvironmentNode
global uniform float maszyna_light_level;
global uniform float maszyna_overcast;

instance uniform vec3 light_color : source_color = vec3(1.0);
instance uniform float cos_hotspot_angle = 1.0;
instance uniform float cos_falloff_angle = 0.0;
instance uniform float max_distance = 15000.0;
// fLight: the glare shows below this light level
instance uniform float lights_on_threshold = 0.0;

// fx/lightglare, bound by MaterialManager
uniform sampler2D glare_texture : source_color, filter_linear_mipmap;
uniform sampler2D depth_texture : hint_depth_texture, filter_nearest;

#include "res://addons/libmaszyna/legacy/materials/types/free_spotlight.gdshaderinc"

// half of the billboard's 2.5 m (opengl33renderer.cpp:110)
const float GLARE_HALF_SIZE = 1.25;
// the glare fades out as the light level rises to this (opengl33renderer.cpp:4390)
const float GLARE_LUMINANCE = 0.6;
// above this overcast it rains or snows, and the glare shows whatever the light level
const float GLARE_OVERCAST = 1.0;

varying float v_glare;
varying vec4 v_center_clip;

void vertex() {
	float square_distance = free_spotlight_square_distance(MODELVIEW_MATRIX);
	vec4 center_view = free_spotlight_center(MODELVIEW_MATRIX, square_distance, max_distance);
	float angle_factor = free_spotlight_angle_factor(MODELVIEW_MATRIX, center_view, cos_hotspot_angle, cos_falloff_angle);
	v_glare = 0.0;
	if (free_spotlight_in_range(square_distance, max_distance)
			&& (maszyna_light_level < lights_on_threshold || maszyna_overcast > GLARE_OVERCAST)) {
		v_glare = clamp(max(GLARE_LUMINANCE - maszyna_light_level, maszyna_overcast - GLARE_OVERCAST), 0.0, 1.0)
				* angle_factor;
	}
	// glRotated(atan2(x, z)) about the vertical
	float yaw = atan(center_view.x, center_view.z);
	vec3 right = vec3(cos(yaw), 0.0, -sin(yaw));
	vec3 corner = center_view.xyz + (right * VERTEX.x + vec3(0.0, 1.0, 0.0) * VERTEX.y) * GLARE_HALF_SIZE;
	POSITION = PROJECTION_MATRIX * vec4(corner, 1.0);
	v_center_clip = PROJECTION_MATRIX * center_view;
	UV = vec2(0.5 - VERTEX.x * 0.5, 0.5 + VERTEX.y * 0.5);
}

void fragment() {
	if (v_glare <= 0.0) {
		discard;
	}
	vec3 center_ndc = v_center_clip.xyz / v_center_clip.w;
	vec2 center_uv = center_ndc.xy * 0.5 + 0.5;
	if (v_center_clip.w <= 0.0 || any(lessThan(center_uv, vec2(0.0))) || any(greaterThan(center_uv, vec2(1.0)))) {
		discard;
	}
	// reversed depth: the scene is in front of the light when its depth is greater
	if (texture(depth_texture, center_uv).r > center_ndc.z) {
		discard;
	}
	vec4 glare = texture(glare_texture, UV);
	ALBEDO = glare.rgb * light_color;
	ALPHA = glare.a * v_glare;
}
