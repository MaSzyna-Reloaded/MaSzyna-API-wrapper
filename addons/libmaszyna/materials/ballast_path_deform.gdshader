shader_type spatial;
render_mode cull_disabled;

instance uniform float path_length;
instance uniform int path_point_count = 0;

uniform vec4 path_points[256]; // xyz: pos, w: tilt
uniform vec4 path_quats[256];  // quaternion orientation
uniform vec4 albedo : source_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D albedo_tex : source_color, hint_default_white, repeat_enable;
uniform float uv_scale = 1.0;

vec3 quat_mul_vec3(vec4 q, vec3 v) {
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void vertex() {
	if (path_point_count < 2) {
		VERTEX = vec3(0.0);
	} else {
		float unit_dist = clamp(VERTEX.z / path_length, 0.0, 1.0);
		float float_idx = unit_dist * float(path_point_count - 1);
		int idx = int(floor(float_idx));
		int next_idx = min(idx + 1, path_point_count - 1);
		float weight = fract(float_idx);
		
		vec4 p1 = path_points[idx];
		vec4 p2 = path_points[next_idx];
		vec3 pos = mix(p1.xyz, p2.xyz, weight);
		float tilt = mix(p1.w, p2.w, weight);
		
		vec4 q1 = path_quats[idx];
		vec4 q2 = path_quats[next_idx];
		if (dot(q1, q2) < 0.0) q2 = -q2;
		vec4 q = normalize(mix(q1, q2, weight));
		
		vec3 fwd = quat_mul_vec3(q, vec3(0.0, 0.0, 1.0));
		vec3 up = quat_mul_vec3(q, vec3(0.0, 1.0, 0.0));
		vec3 side = quat_mul_vec3(q, vec3(1.0, 0.0, 0.0));
		
		vec3 tilted_up = up * cos(tilt) + side * sin(tilt);
		vec3 tilted_side = side * cos(tilt) - up * sin(tilt);
		
		vec3 local_pos = pos + (tilted_side * VERTEX.x) + (tilted_up * VERTEX.y);
		VERTEX = local_pos;
		
		NORMAL = normalize((tilted_side * NORMAL.x) + (tilted_up * NORMAL.y) + (fwd * NORMAL.z));
	}
}

void fragment() {
	ALBEDO = albedo.rgb * texture(albedo_tex, UV * uv_scale).rgb;
	METALLIC = 0.0;
	ROUGHNESS = 0.8;
}
