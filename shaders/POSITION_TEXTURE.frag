#pragma once

const char POSITION_TEXTURE_frag[] =
R"($input v_texcoord0

SAMPLER2D(u_texture, 0);

void main()
{
    gl_FragColor = texture2D(u_texture, v_texcoord0);
}
)";
