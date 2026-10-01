shader_type canvas_item;

// The load's progress as an arc round a faint ring, and inside it a comet that never stops
// turning - the ring tells how far the load is, the comet that it is still going while the arc
// stands. Spun by TIME, so nothing runs on the CPU per frame.

uniform float progress : hint_range(0.0, 1.0) = 0.0;
uniform vec4 track_color : source_color = vec4(0.81, 0.88, 1.0, 0.12);
uniform vec4 value_color : source_color = vec4(0.45, 0.72, 1.0, 1.0);
uniform vec4 comet_color : source_color = vec4(1.0, 1.0, 1.0, 0.8);
uniform float turns_per_second = 0.9;
// Radii and widths as fractions of the half size
uniform float ring_radius = 0.84;
uniform float ring_width = 0.15;
uniform float comet_radius = 0.56;
uniform float comet_width = 0.09;
// Share of the inner ring the comet's tail covers
uniform float tail_length : hint_range(0.05, 1.0) = 0.35;

vec4 over(vec4 back, vec4 front) {
    return vec4(mix(back.rgb, front.rgb, front.a), front.a + back.a * (1.0 - front.a));
}

void fragment() {
    vec2 p = UV * 2.0 - 1.0;
    float r = length(p);
    float aa = fwidth(r);
    // 0 at the top, growing clockwise
    float angle = fract(atan(p.x, -p.y) / TAU);

    float ring = 1.0 - smoothstep(ring_width * 0.5 - aa, ring_width * 0.5, abs(r - ring_radius));
    vec4 color = vec4(track_color.rgb, track_color.a * ring);
    float value = ring * (1.0 - smoothstep(progress - aa, progress, angle));
    color = over(color, vec4(value_color.rgb, value_color.a * value));

    float comet = 1.0 - smoothstep(comet_width * 0.5 - aa, comet_width * 0.5, abs(r - comet_radius));
    float behind = fract(fract(TIME * turns_per_second) - angle);
    float tail = 1.0 - smoothstep(0.0, tail_length, behind);
    color = over(color, vec4(comet_color.rgb, comet_color.a * comet * tail * tail));

    COLOR = color * COLOR;
}
