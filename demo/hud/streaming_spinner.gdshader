shader_type canvas_item;

// A comet running round a ring, nothing behind it. The spinning is driven by TIME, so nothing runs
// on the CPU per frame.

uniform vec4 comet_color : source_color = vec4(0.45, 0.72, 1.0, 1.0);
uniform float turns_per_second = 0.9;
// Radius and width as fractions of the half size
uniform float ring_radius = 0.76;
uniform float ring_width = 0.24;
// Share of the ring the comet's tail covers
uniform float tail_length : hint_range(0.05, 1.0) = 0.7;

void fragment() {
    vec2 p = UV * 2.0 - 1.0;
    float r = length(p);
    float aa = fwidth(r);
    float half_width = ring_width * 0.5;
    float ring = 1.0 - smoothstep(half_width - aa, half_width, abs(r - ring_radius));

    // 0 at the comet's head, growing behind it; the tail fades out with a soft curve
    float head = fract(TIME * turns_per_second) * TAU;
    float behind = fract((head - atan(p.y, p.x)) / TAU);
    float tail = 1.0 - smoothstep(0.0, tail_length, behind);
    tail *= tail;
    // the head is a round cap, not a cut-off end of the ring
    vec2 head_position = ring_radius * vec2(cos(head), sin(head));
    float cap = 1.0 - smoothstep(half_width - aa, half_width, length(p - head_position));

    COLOR = vec4(comet_color.rgb, comet_color.a * max(ring * tail, cap)) * COLOR;
}
