#pragma once

const char ETC1_GRAY_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);
SAMPLER2D(u_texture1, 1);

void main()
{
    vec4 texColor = texture2D(u_texture, v_texcoord0);
    texColor.a = texture2D(u_texture1, v_texcoord0).r;
    texColor.rgb *= texColor.a; // premultiply alpha channel

    texColor = v_color0 * texColor;

    gl_FragColor.rgb = vec3_splat(0.2126*texColor.r + 0.7152*texColor.g + 0.0722*texColor.b);
    gl_FragColor.a = texColor.a;
}
)";
