shader_type canvas_item;

// A dark lamp of the driving aid, greyed out where the scene behind it is dark: over the dark cab
// its dark body would vanish, so it is lifted towards grey; over the sky it stays dark, as the
// tile's plate under it is. Only its colour changes, its alpha (and so the rounded corners) not.

uniform sampler2D screen_texture : hint_screen_texture, filter_linear_mipmap;
// The background brightness the lamp is fully lifted below, and not lifted above
uniform float dark_from = 0.15;
uniform float dark_to = 0.45;
// How far the lamp's colour is lifted towards white over the darkest background
uniform float lift = 0.45;
uniform float background_lod = 2.0;

void fragment() {
	vec3 background = textureLod(screen_texture, SCREEN_UV, background_lod).rgb;
	float brightness = dot(background, vec3(0.299, 0.587, 0.114));
	float darkness = 1.0 - smoothstep(dark_from, dark_to, brightness);
	COLOR.rgb = mix(COLOR.rgb, vec3(1.0), lift * darkness);
}
