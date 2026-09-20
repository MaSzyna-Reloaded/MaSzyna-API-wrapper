// Windshield rain effects, ported from the original mat_rain_windscreen.frag + random.glsl
// (by @lcddisplay; wiper movement @MichauSto).

shader_type spatial;
render_mode blend_mix, cull_back, diffuse_burley, specular_schlick_ggx;

uniform sampler2D diffuse_texture : source_color, filter_linear_mipmap, repeat_enable;
// 2x2 atlas of droplet sprites
uniform sampler2D raindrops_atlas : source_color, filter_linear_mipmap, repeat_enable;
// one wiper per channel: the phase of its sweep at which the blade passes the texel, 0 outside of its area
uniform sampler2D wiper_mask : hint_default_black, filter_linear, repeat_enable;

// stands in for the "ambient" of the original, see fragment()
uniform sampler2D screen_texture : hint_screen_texture, filter_linear_mipmap, repeat_disable;

uniform float raindrop_grid_size = 1.0;
uniform float specular : hint_range(0.0, 1.0, 0.01) = 0.5;
uniform float roughness : hint_range(0.0, 1.0) = 0.2;

// A droplet takes a few pixels of the screen and a 512 px quadrant of the atlas: the mip level
// picked for it averages the rim away and its alpha falls below the 0.3 of sharp_alpha.
uniform float drop_lod_bias = -2.0;
uniform float specular_intensity = 1.0;
uniform float wobble_strength = 0.002;
uniform float wobble_speed = 30.0;

// rain_params.x / rain_params.y, wiper_pos, wiper_timer_out, wiper_timer_return of the original
// scene uniforms (opengl33renderer.cpp:750-790)
global uniform float maszyna_rain_intensity;
global uniform float maszyna_wiper_regen_time;
// per wiper: 0..1 sweeping out, 1..2 returning
global uniform vec4 maszyna_wiper_pos;
// per wiper: TIME of its last pass out and back
global uniform vec4 maszyna_wiper_timer_out;
global uniform vec4 maszyna_wiper_timer_return;

// Evolving Sub-Grid Turbulence for Smoke Animation, H. Schechter and R. Bridson (random.glsl)
uint hash(uint s) {
    s ^= 2747636419u;
    s *= 2654435769u;
    s ^= s >> 16u;
    s *= 2654435769u;
    s ^= s >> 16u;
    s *= 2654435769u;
    return s;
}

uint hash_combine(uint seed, uint value) {
    return seed ^ (hash(value) + 0x9e3779b9u + (seed << 6u) + (seed >> 2u));
}

uint hash_cell(ivec2 cell, uint side) {
    return hash_combine(hash_combine(hash(uint(cell.x)), uint(cell.y)), side);
}

float rand_f(inout uint seed) {
    seed = hash(seed);
    return float(seed) * 2.3283064365386963e-10;
}

vec4 get_drop_tex(float choice, vec2 uv) {
    vec2 offset;
    if (choice < 0.25) {
        offset = vec2(0.0, 0.0);
    } else if (choice < 0.5) {
        offset = vec2(0.5, 0.0);
    } else if (choice < 0.75) {
        offset = vec2(0.0, 0.5);
    } else {
        offset = vec2(0.5, 0.5);
    }
    return texture(raindrops_atlas, offset + uv * 0.5, drop_lod_bias);
}

// How much of the rain is left at the given point after the wipers; side tells the two halves of
// a wiper's area apart, so that each regrows its own droplets.
float get_mix_factor(vec2 co, out float side) {
    vec4 move_phase = maszyna_wiper_pos;
    vec4 is_out = vec4(lessThanEqual(move_phase, vec4(1.0)));
    move_phase = mix(vec4(2.0) - move_phase, move_phase, is_out);

    vec4 mask = texture(wiper_mask, co);
    vec4 area_mask = step(0.001, mask);

    vec4 mask_val = mix(1.0 - mask, mask, is_out);
    vec4 wipe_width = smoothstep(1.0, 0.9, move_phase) * 0.25;
    vec4 cleaned = smoothstep(move_phase - wipe_width, move_phase, mask_val) * area_mask;
    vec4 side_v = step(mask_val, move_phase);
    cleaned *= side_v;
    side_v = mix(side_v, vec4(1.0) - side_v, is_out);

    // "regeneration", raindrops gradually returning after wiper pass
    vec4 regen_phase = clamp(
            (vec4(TIME) - mix(maszyna_wiper_timer_out, maszyna_wiper_timer_return, side_v) - vec4(0.2))
                    / vec4(maszyna_wiper_regen_time),
            vec4(0.0), vec4(1.0));

    side_v = mix(vec4(0.0), vec4(1.0) - side_v, area_mask);

    vec4 factor_v = mix(vec4(1.0), regen_phase * (vec4(1.0) - cleaned), area_mask);

    side = 0.0;
    float out_factor = 1.0;

    // Find out the wiper blade that influences given grid cell the most
    for (int i = 0; i < 4; ++i) {
        if (factor_v[i] < out_factor) {
            out_factor = factor_v[i];
            side = side_v[i];
        }
    }

    return out_factor;
}

void fragment() {
    vec4 tex_color = texture(diffuse_texture, UV);
    if (tex_color.a < 0.01) {
        discard;
    }

    float grid_size = ceil(raindrop_grid_size);
    const float cycle_duration = 4.0;

    float square_min = 0.5 / grid_size;
    float square_max = 1.2 / grid_size;

    vec2 cell = floor(UV * grid_size);

    // The droplets of the original are not lit: their colour (a white rim in the atlas) is scaled
    // by the luminance of the ambient light only. A fragment shader has no ambient here, and lit
    // through ALBEDO the rims go black inside of a dark cab - so the droplets are emitted, scaled
    // by the blurred luminance of what is seen through the glass.
    float amb_lum = clamp(dot(textureLod(screen_texture, SCREEN_UV, 6.0).rgb, vec3(0.2126, 0.7152, 0.0722)), 0.0, 1.0);
    float sun_factor = pow(clamp(amb_lum * 6.0, 0.0, 1.0), 0.5);

    vec3 drop_layer = vec3(0.0);
    float drop_mask_sum = 0.0;

    // Grid of 9 droplets in immediate neighbourhood
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            vec2 neighbor_cell = cell + vec2(float(ox), float(oy));
            vec2 neighbor_center = (neighbor_cell + 0.5) / grid_size;

            float side;
            float mix_factor = get_mix_factor(neighbor_center, side);

            uint seed = hash_cell(ivec2(neighbor_cell), uint(side));

            if (mix_factor < rand_f(seed)) {
                continue;
            }

            // Show a percentage of droplets given by rain intensity
            if (rand_f(seed) > maszyna_rain_intensity) {
                continue;
            }

            // Randomly modulate droplet center & size
            vec2 drop_center = (neighbor_cell + vec2(rand_f(seed), rand_f(seed))) / grid_size;
            float square_size = mix(square_min, square_max, rand_f(seed));

            float life_time = TIME + rand_f(seed) * cycle_duration;
            float phase = fract(life_time / cycle_duration);
            float active = clamp(1.0 - phase, 0.0, 1.0);

            // Gravity influence
            float gravity_phase = smoothstep(0.5, 1.0, phase);
            float drop_mass = mix(0.3, 1.2, rand_f(seed));
            float gravity_speed = 0.15 * drop_mass;
            vec2 gravity_offset = vec2(0.0, gravity_phase * gravity_speed * phase);

            // Random wobble
            bool has_wobble = rand_f(seed) < 0.10;
            vec2 wobble_offset = vec2(0.0);
            if (has_wobble && gravity_phase > 0.0) {
                float intensity = sin(TIME * wobble_speed + rand_f(seed) * 100.0) * wobble_strength * gravity_phase;
                wobble_offset = vec2(intensity, 0.0);
            }

            vec2 slide_offset = gravity_offset + wobble_offset;

            // Flatten droplets influenced by gravity
            float flatten_amount = smoothstep(0.1, 0.5, gravity_phase);
            float flatten_x = mix(1.0, 0.4, flatten_amount);
            float stretch_y = mix(1.0, 1.6, flatten_amount);

            // Droplet local position & mask
            vec2 diff = (UV + slide_offset) - drop_center;
            diff.x *= 1.0 / flatten_x;
            diff.y *= 1.0 / stretch_y;
            float mask = smoothstep(square_size * 0.5, square_size * 0.45, max(abs(diff.x), abs(diff.y)));

            if (mask > 0.001) {
                vec2 local_uv = (diff + square_size * 0.5) / square_size;
                float choice = rand_f(seed);
                vec4 drop_tex = get_drop_tex(choice, local_uv);
                float sharp_alpha = smoothstep(0.3, 0.9, drop_tex.a);

                float color_luma = length(drop_tex.rgb);
                float alpha_range = smoothstep(0.1, 0.3, color_luma);
                float black_alpha = mix(0.25, 0.85, alpha_range);

                float sparkle = 0.0;
                if (has_wobble) {
                    sparkle = pow(abs(sin(UV.x * 8.0 + TIME * 2.0 + rand_f(seed) * 6.2831)), 40.0)
                            * mix(0.2, 1.0, sun_factor);
                }

                vec3 drop_lit = drop_tex.rgb * sun_factor + vec3(specular_intensity * sparkle) * sharp_alpha;

                drop_layer += drop_lit * sharp_alpha * active * black_alpha * mask;
                drop_mask_sum += sharp_alpha * active * black_alpha * mask;
            }
        }
    }

    // Overlay windshield texture with alpha
    ALBEDO = tex_color.rgb * tex_color.a;
    EMISSION = drop_layer * (1.0 - tex_color.a);
    ALPHA = mix(clamp(drop_mask_sum, 0.0, 1.0), 1.0, tex_color.a);
    SPECULAR = specular;
    ROUGHNESS = roughness;
    METALLIC = 0.0;
}
