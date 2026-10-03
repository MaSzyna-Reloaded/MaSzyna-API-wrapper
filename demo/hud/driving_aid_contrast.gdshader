shader_type canvas_item;

// The background of a driving aid tile, darker where the scene behind it is bright: over the dark
// cab it stays faint, over the sky it becomes a dark plate, so the light text on it reads on both.
// It only scales the tile's own background alpha, which keeps the anti-aliased rounded corners.

uniform sampler2D screen_texture : hint_screen_texture, filter_linear_mipmap;
// The background brightness the plate starts to darken at, and where it is darkest
uniform float bright_from = 0.3;
uniform float bright_to = 0.8;
// The plate's alpha over the brightest background, as a multiple of the tile's own
uniform float bright_alpha_scale = 7.0;
// The screen's mip level the brightness is read from, softening the plate at a wire's edge
uniform float background_lod = 2.0;

void fragment() {
	vec3 background = textureLod(screen_texture, SCREEN_UV, background_lod).rgb;
	float brightness = dot(background, vec3(0.299, 0.587, 0.114));
	COLOR.a *= mix(1.0, bright_alpha_scale, smoothstep(bright_from, bright_to, brightness));
}
