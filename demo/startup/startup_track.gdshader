shader_type canvas_item;

// Spinner of the startup screen, drawn in the 520x240 box of the EU07 sketch: the loco stands,
// the track and the wire scroll under it (layer 0, behind the loco) and once in a while a
// catenary pole passes in front of it (layer 1).

uniform int layer = 0;
uniform vec4 line_color : source_color = vec4(0.81, 0.88, 1.0, 1.0);
uniform vec2 box_size = vec2(520.0, 240.0);
uniform float rail_y = 218.0;
uniform float wire_y = 61.0;
uniform float speed = 180.0; // px/s
uniform float sleeper_spacing = 23.0;
uniform float pole_period = 4.0; // s
uniform float pole_speed = 300.0; // px/s, faster than the track - closer to the viewer

float band(float value, float center, float half_width) {
	return 1.0 - smoothstep(half_width, half_width + 1.0, abs(value - center));
}

void fragment() {
	vec2 p = UV * box_size;
	float a = 0.0;
	if (layer == 0) {
		a = max(a, band(p.y, rail_y, 1.5));
		float sleeper_x = mod(p.x + TIME * speed, sleeper_spacing);
		a = max(a, band(sleeper_x, 3.0, 2.5) * band(p.y, rail_y + 5.5, 2.5));
		a = max(a, 0.8 * band(p.y, wire_y, 0.8));
	} else {
		float pole_x = box_size.x + 30.0 - mod(TIME, pole_period) * pole_speed;
		float mast = band(p.x, pole_x, 4.0) * step(wire_y - 30.0, p.y);
		float arm = band(p.y, wire_y - 12.0, 1.5) * step(pole_x - 44.0, p.x) * step(p.x, pole_x);
		a = max(mast, arm);
	}
	// fade out at the box sides
	a *= smoothstep(0.0, 0.12, UV.x) * smoothstep(1.0, 0.88, UV.x);
	// input COLOR carries the node modulate (fade out)
	COLOR *= vec4(line_color.rgb, line_color.a * a);
}
