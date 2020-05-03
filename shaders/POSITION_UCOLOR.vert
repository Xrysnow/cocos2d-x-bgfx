#pragma once

const char POSITION_UCOLOR_vert[] =
R"($input a_position
$output v_color0

uniform mat4 u_MVPMatrix;
uniform vec4 u_color;

void main()
{
    gl_Position = mul(u_MVPMatrix, vec4(a_position, 0.0, 1.0));
    gl_Position.xy = applyVP(gl_Position.xy);
    v_color0 = u_color;
}
)";
