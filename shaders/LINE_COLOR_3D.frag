#pragma once

const char LINE_COLOR_3D_frag[] =
R"($input v_color0

void main()
{
    gl_FragColor = v_color0;
}
)";
