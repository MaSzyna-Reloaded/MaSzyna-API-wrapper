shader_type canvas_item;

// Loading screen background toned dark blue (like sepia, in blue) and dissolved away by
// noise blotches when a scenario is chosen, uncovering the loading screen below.

uniform vec3 tint : source_color = vec3(0.16, 0.27, 0.5);
uniform float brightness : hint_range(0.0, 2.0) = 1.2;
// 0 - fully visible, 1 - fully dissolved
uniform float dissolve : hint_range(0.0, 1.0) = 0.0;
uniform float dissolve_softness : hint_range(0.01, 1.0) = 0.25;
uniform float dissolve_scale = 6.0;

float hash(vec2 p) {
	return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float value_noise(vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),
			mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

void fragment() {
	vec4 color = texture(TEXTURE, UV);
	float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));
	float n = 0.65 * value_noise(UV * dissolve_scale) + 0.35 * value_noise(UV * dissolve_scale * 4.0);
	float threshold = dissolve * (1.0 + dissolve_softness);
	COLOR = vec4(tint * luminance * brightness, color.a * smoothstep(threshold - dissolve_softness, threshold, n));
}
