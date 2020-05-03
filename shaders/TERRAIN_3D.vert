#pragma once

const char TERRAIN_3D_vert[] =
R"($input a_position, a_normal, a_texcoord0
$output v_texcoord0, v_normal

uniform mat4 u_MVPMatrix;

void main()
{
    gl_Position = mul(u_MVPMatrix, vec4(a_position, 1.0));
    gl_Position.xy = applyVP(gl_Position.xy);
    v_texcoord0 = a_texcoord0;
    v_normal = a_normal;
}
)";
