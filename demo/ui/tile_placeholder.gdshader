shader_type canvas_item;

// Tile whose side view is still being rendered: the silhouette of a vehicle, faint, with a spinner
// turning over it. Both are drawn here, so a tile that waits costs no script and no node ticking.

uniform sampler2D silhouette: source_color, filter_linear;
/// Size of the tile in pixels - a round spinner cannot be drawn in the UV of a wide rectangle
uniform vec2 rect_size = vec2(150.0, 81.0);
uniform vec4 silhouette_color: source_color = vec4(0.72, 0.76, 0.82, 0.22);
uniform vec4 spinner_color: source_color = vec4(0.7, 0.82, 1.0, 0.9);
/// Turns per second
uniform float spinner_speed = 0.9;
uniform float spinner_radius = 11.0;
uniform float spinner_thickness = 2.5;
/// Breathing room around the silhouette, as a fraction of the tile on every side
uniform float silhouette_padding = 0.12;

void fragment() {
    // the silhouette is drawn into the tile shrunk by the padding, not stretched over all of it
    vec2 art_uv = (UV - silhouette_padding) / (1.0 - 2.0 * silhouette_padding);
    vec2 inside = step(vec2(0.0), art_uv) * step(art_uv, vec2(1.0));
    float art = texture(silhouette, art_uv).a * silhouette_color.a * inside.x * inside.y;

    vec2 offset = UV * rect_size - rect_size * 0.5;
    // the ring, a couple of pixels thick, and the head of the comet running around it
    float ring = clamp(spinner_thickness - abs(length(offset) - spinner_radius), 0.0, 1.0);
    float head = fract(atan(offset.y, offset.x) / TAU - TIME * spinner_speed);
    float spinner = ring * pow(head, 2.5) * spinner_color.a;

    COLOR = vec4(mix(silhouette_color.rgb, spinner_color.rgb, spinner), max(art, spinner));
}
