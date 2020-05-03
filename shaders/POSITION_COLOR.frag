#pragma once

const char POSITION_COLOR_frag[] =
R"($input v_color0

void main()
{
    gl_FragColor = v_color0;
}
)";
