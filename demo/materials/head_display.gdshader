shader_type spatial;

uniform float scroll_speed : hint_range(-10.0, 10.0) = 0.2; // Speed of scrolling
uniform vec4 text_color : source_color = vec4(1.0, 1.0, 1.0, 1.0); // Text color
uniform vec2 scale = vec2(1.0, 1.0);
uniform sampler2D text_texture : hint_default_black;
uniform vec2 offset = vec2(0.0, 0.0);

void fragment() {
    vec2 uv = UV;
    uv.x += TIME * scroll_speed;

    uv = fract(uv * scale) + offset;
    vec4 tex_color = texture(text_texture, uv);
    ALBEDO.rgb = (tex_color * text_color).rgb;
}
