#pragma once

const char PARTICLE_COLOR_3D_frag[] =
R"($input ColorOut

uniform vec4 u_color;

void main()
{
    gl_FragColor = ColorOut * u_color;
}
)";
