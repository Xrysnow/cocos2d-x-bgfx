#pragma once

const char LABEL_NORMAL_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);
uniform vec4 u_textColor;

void main()
{
    gl_FragColor =  v_color0 * vec4(u_textColor.rgb,// RGB from uniform
        u_textColor.a * texture2D(u_texture, v_texcoord0).a// A from texture & uniform
    );
}
)";
