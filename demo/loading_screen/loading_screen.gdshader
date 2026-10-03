shader_type canvas_item;

// Xor's gaussian blur (https://xorshaders.weebly.com/tutorials/blur-shaders-5-part-2, defaults
// from https://www.shadertoy.com/view/Xltfzj), written into fragment() itself: TEXTURE is a
// built-in, and handed to a function as a sampler argument it fails to compile.

// the full turn the samples go round, as the original writes it
const float FULL_TURN = 6.28;
// directions and rings of samples - more is better but slower
const int ITERATIONS = 16;
const int QUALITY = 4;

uniform int blur_amount = 16;

void fragment(){
	// input COLOR carries the node modulate (fade out)
	vec4 modulate = COLOR;
	vec2 uv = FRAGCOORD.xy / (1.0 / SCREEN_PIXEL_SIZE).xy;
	vec2 radius = float(blur_amount) / (1.0 / SCREEN_PIXEL_SIZE).xy;
	vec4 blurred = texture(TEXTURE, uv);
	for (float d = 0.0; d < FULL_TURN; d += FULL_TURN / float(ITERATIONS)) {
		for (float i = 1.0 / float(QUALITY); i <= 1.0; i += 1.0 / float(QUALITY)) {
			blurred += texture(TEXTURE, uv + vec2(cos(d), sin(d)) * radius * i);
		}
	}
	blurred /= float(QUALITY) * float(ITERATIONS) + 1.0;
	COLOR = modulate * blurred;
}
