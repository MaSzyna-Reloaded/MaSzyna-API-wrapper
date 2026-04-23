shader_type spatial;
render_mode depth_prepass_alpha;

instance uniform float path_length;
instance uniform float sleeper_spacing = 1.0;
instance uniform float sleeper_height = 0.0;
instance uniform int path_point_count = 0;

uniform vec4 path_points[256]; // xyz: pos, w: tilt
uniform vec4 path_quats[256];  // quaternion orientation
uniform vec4 albedo : source_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D albedo_tex : source_color, hint_default_white;

vec3 quat_mul_vec3(vec4 q, vec3 v) {
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void vertex() {
	if (path_point_count < 2) {
		VERTEX = vec3(0.0);
	} else {
		// Instance index gives us which sleeper this is
		float dist = float(INSTANCE_ID) * sleeper_spacing;
		float unit_dist = clamp(dist / path_length, 0.0, 1.0);
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
		// Cheap nlerp for quaternions
		if (dot(q1, q2) < 0.0) q2 = -q2;
		vec4 q = normalize(mix(q1, q2, weight));
		
		vec3 fwd = quat_mul_vec3(q, vec3(0.0, 0.0, 1.0));
		vec3 up = quat_mul_vec3(q, vec3(0.0, 1.0, 0.0));
		vec3 side = quat_mul_vec3(q, vec3(1.0, 0.0, 0.0));
		
		// Reconstruct the tilted basis
		vec3 tilted_up = up * cos(tilt) + side * sin(tilt);
		vec3 tilted_side = side * cos(tilt) - up * sin(tilt);
		
		// Create rotation matrix from basis
		// We use -tilted_side and -fwd to rotate 180 degrees around Y (tilted_up)
		// This keeps the basis right-handed (det = 1) and winding order correct.
		mat3 basis = mat3(-tilted_side, tilted_up, -fwd);
		
		// Transform vertex
		// VERTEX is in sleeper local space
		vec3 local_v = basis * VERTEX;
		
		// Add path position and height offset
		vec3 world_pos = pos + local_v + tilted_up * sleeper_height;
		
		// Output the calculated local vertex to Godot
		VERTEX = world_pos;
		
		// Update normal
		NORMAL = normalize(basis * NORMAL);
		TANGENT = normalize(basis * TANGENT);
		BINORMAL = normalize(basis * BINORMAL);
	}
}

void fragment() {
	vec4 tex = texture(albedo_tex, UV);
	ALBEDO = albedo.rgb * tex.rgb;
	ALPHA = albedo.a * tex.a;
}
