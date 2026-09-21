shader_type canvas_item;

// Marker of the selected row: a triangle pointing right, blinking on its own. The blink is the
// shader's clock and nothing else - no script runs per frame for it.

uniform vec4 marker_color: source_color = vec4(0.45, 0.72, 1.0, 1.0);
/// Full on-off cycles per second
uniform float blink_hz: hint_range(0.5, 10.0) = 3.0;

void fragment() {
    // base at the left edge, apex at the right: |y - 0.5| under (1 - x) * 0.5
    float edge = (1.0 - UV.x) * 0.5 - abs(UV.y - 0.5);
    float shape = smoothstep(0.0, 0.04, edge);
    float blink = step(0.5, fract(TIME * blink_hz));
    COLOR = vec4(marker_color.rgb, marker_color.a * shape * blink);
}
