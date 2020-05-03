#pragma once

const char CAMERA_CLEAR_frag[] =
R"($input v_color0, v_texcoord0

void main()
{
    gl_FragColor = v_color0;
}
)";
