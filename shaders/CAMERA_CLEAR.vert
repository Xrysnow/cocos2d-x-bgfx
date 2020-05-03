#pragma once

const char CAMERA_CLEAR_vert[] =
R"($input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

uniform float depth;

void main()
{
    gl_Position.xy = a_position.xy;
    gl_Position.xy = applyVP(gl_Position.xy);
    gl_Position.z = depth;
    gl_Position.w = 1.0;
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
}
)";
