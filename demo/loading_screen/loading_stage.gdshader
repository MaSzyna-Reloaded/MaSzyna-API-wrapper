shader_type canvas_item;

// One stage of the load as a bar: a stage to come is faint, a stage done is filled, and the stage
// under way carries a light sweeping along it - so it shows that it is going even while the load's
// progress stands. Moved by TIME alone.

uniform float done : hint_range(0.0, 1.0) = 0.0;
uniform float active : hint_range(0.0, 1.0) = 0.0;
uniform vec4 idle_color : source_color = vec4(0.81, 0.88, 1.0, 0.16);
uniform vec4 done_color : source_color = vec4(0.45, 0.72, 1.0, 1.0);
uniform vec4 light_color : source_color = vec4(1.0, 1.0, 1.0, 0.7);
uniform float sweeps_per_second = 0.75;

void fragment() {
    vec4 color = mix(idle_color, done_color, done);
    float sweep = fract(TIME * sweeps_per_second) * 1.6 - 0.3;
    float light = 1.0 - smoothstep(0.0, 0.18, abs(UV.x - sweep));
    color = mix(color, vec4(mix(idle_color.rgb, light_color.rgb, light), max(idle_color.a, light_color.a * light)), active);
    COLOR = color * COLOR;
}
