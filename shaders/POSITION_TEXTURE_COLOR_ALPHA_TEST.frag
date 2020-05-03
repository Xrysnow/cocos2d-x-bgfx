#pragma once

const char POSITION_TEXTURE_COLOR_ALPHA_TEST_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);
uniform float u_alpha_value;

void main()
{
    vec4 texColor = texture2D(u_texture, v_texcoord0);
// mimic: glAlphaFunc(GL_GREATER)
// pass if ( incoming_pixel >= u_alpha_value ) => fail if incoming_pixel < u_alpha_value
    if ( texColor.a <= u_alpha_value )
        discard;
    gl_FragColor = texColor * v_color0;
}
)";
