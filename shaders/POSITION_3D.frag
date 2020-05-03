#pragma once

const char POSITION_3D_frag[] =
R"($input TextureCoordOut

uniform vec4 u_color;

void main()
{
    gl_FragColor = u_color;
}
)";
