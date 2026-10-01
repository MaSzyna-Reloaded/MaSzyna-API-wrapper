shader_type canvas_item;

// One stage of the load: a bar (mark = false) or a round mark (mark = true). A stage to come is
// a faint outline, a stage done is filled, and the stage under way carries a moving light - so it
// shows that it is going even while the load's progress stands. Moved by TIME alone.

uniform bool mark = false;
uniform float done : hint_range(0.0, 1.0) = 0.0;
uniform float active : hint_range(0.0, 1.0) = 0.0;
uniform vec4 idle_color : source_color = vec4(0.81, 0.88, 1.0, 0.16);
uniform vec4 done_color : source_color = vec4(0.45, 0.72, 1.0, 1.0);
uniform vec4 light_color : source_color = vec4(1.0, 1.0, 1.0, 0.7);
uniform float sweeps_per_second = 0.75;
// Outline of a mark, as a fraction of its half size
uniform float outline = 0.2;

void fragment() {
    vec4 color;
    if (mark) {
        vec2 p = UV * 2.0 - 1.0;
        float r = length(p);
        float aa = fwidth(r);
        float disc = 1.0 - smoothstep(1.0 - aa, 1.0, r);
        float rim = disc * smoothstep(1.0 - outline - aa, 1.0 - outline, r);
        color = mix(vec4(idle_color.rgb, idle_color.a * rim), vec4(done_color.rgb, done_color.a * disc), done);
        // under way: a quarter of the rim running round
        float angle = fract(atan(p.x, -p.y) / TAU);
        float arc = 1.0 - smoothstep(0.0, 0.3, fract(fract(TIME * sweeps_per_second) - angle));
        color = mix(color, vec4(done_color.rgb, done_color.a * rim * arc), active);
    } else {
        color = mix(idle_color, done_color, done);
        // under way: a light sweeping along the bar
        float sweep = fract(TIME * sweeps_per_second) * 1.6 - 0.3;
        float light = 1.0 - smoothstep(0.0, 0.18, abs(UV.x - sweep));
        color = mix(color, vec4(mix(idle_color.rgb, light_color.rgb, light), max(idle_color.a, light_color.a * light)), active);
    }
    COLOR = color * COLOR;
}
