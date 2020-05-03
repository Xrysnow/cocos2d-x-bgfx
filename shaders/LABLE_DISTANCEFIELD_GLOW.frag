#pragma once

const char LABLE_DISTANCEFIELD_GLOW_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);
uniform vec4 u_effectColor;
uniform vec4 u_textColor;

void main()
{
    float dist = texture2D(u_texture, v_texcoord0).a;
    //TODO: Implementation 'fwidth' for glsl 1.0
    //float width = fwidth(dist);
    //assign width for constant will lead to a little bit fuzzy,it's temporary measure.
    float width = 0.04;
    float alpha = smoothstep(0.5-width, 0.5+width, dist);
    //glow
    float mu = smoothstep(0.5, 1.0, sqrt(dist));
    vec4 color = u_effectColor*(1.0-alpha) + u_textColor*alpha;
    gl_FragColor = v_color0 * vec4(color.rgb, max(alpha,mu)*color.a);
}
)";
