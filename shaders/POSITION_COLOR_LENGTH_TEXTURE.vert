#pragma once

const char POSITION_COLOR_LENGTH_TEXTURE_vert[] =
R"($input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

uniform float u_alpha;
uniform mat4 u_MVPMatrix;

void main()
{
    gl_Position = mul(u_MVPMatrix, vec4(a_position, 0.0, 1.0));
    gl_Position.xy = applyVP(gl_Position.xy);
    v_color0 = vec4(a_color0.rgb * a_color0.a * u_alpha, a_color0.a * u_alpha);
    v_texcoord0 = a_texcoord0;
}
)";
