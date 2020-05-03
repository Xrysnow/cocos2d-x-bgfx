#pragma once

const char POSITION_COLOR_LENGTH_TEXTURE_frag[] =
R"($input v_color0, v_texcoord0

void main()
{
    gl_FragColor = v_color0 * step(0.0, 1.0 - length(v_texcoord0));
}
)";
