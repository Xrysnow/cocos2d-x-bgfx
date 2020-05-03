#pragma once

const char ETC1_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);
SAMPLER2D(u_texture1, 1);

void main()
{
    vec4 texColor = vec4(texture2D(u_texture, v_texcoord0).rgb, texture2D(u_texture1, v_texcoord0).r);
    texColor.rgb *= texColor.a; // Premultiply with Alpha channel
    gl_FragColor = v_color0 * texColor;
}
)";
