shader_type canvas_item;

// Background of a skin tile, drawn under the vehicle profile: a solid colour with rounded corners.

uniform vec4 glow_color : source_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float glow_strength : hint_range(0.0, 1.0) = 0.0;
uniform float glow_alpha : hint_range(0.0, 1.0) = 0.1;
uniform vec2 rect_size = vec2(300.0, 108.0);
uniform float corner_radius : hint_range(0.0, 64.0) = 10.0;

void fragment() {
	// distance to a rounded rectangle, in pixels
	vec2 corner = abs(UV * rect_size - rect_size * 0.5) - (rect_size * 0.5 - vec2(corner_radius));
	float distance_outside = length(max(corner, vec2(0.0))) - corner_radius;
	float mask = 1.0 - smoothstep(-1.0, 1.0, distance_outside);
	COLOR = vec4(glow_color.rgb, glow_alpha * glow_strength * mask);
}
