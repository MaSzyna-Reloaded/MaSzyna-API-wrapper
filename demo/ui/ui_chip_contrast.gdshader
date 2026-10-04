shader_type canvas_item;

// The background of a driving aid tile, darker where the scene behind it is bright: over the dark
// cab it stays faint, over the sky it becomes a dark plate, so the light text on it reads on both.
// It only scales the tile's own background alpha, which keeps the anti-aliased rounded corners.
// With HDR output the screen is linear and goes past white (GameWindow sets maszyna_hdr_output): the
// brightness is read as on an ordinary screen - up to white, on the sRGB curve - and the alpha,
// which an 8-bit screen clips at 1, is clipped here, or the blend breaks.

global uniform bool maszyna_hdr_output;

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
	if (maszyna_hdr_output) {
		background = clamp(background, vec3(0.0), vec3(1.0));
		// linear to sRGB, as an ordinary screen holds it
		background = mix(
				background * 12.92,
				1.055 * pow(background, vec3(1.0 / 2.4)) - 0.055,
				step(vec3(0.0031308), background));
	}
	float brightness = dot(background, vec3(0.299, 0.587, 0.114));
	COLOR.a = min(COLOR.a * mix(1.0, bright_alpha_scale, smoothstep(bright_from, bright_to, brightness)), 1.0);
}
