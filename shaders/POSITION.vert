#pragma once

const char POSITION_vert[] =
R"($input a_position, a_color0, a_texcoord0
$output v_position

uniform mat4 u_MVPMatrix;

void main()
{
    gl_Position = mul(u_MVPMatrix, vec4(a_position, 1.0));
    gl_Position.xy = applyVP(gl_Position.xy);
    v_position = a_position;
}
)";
