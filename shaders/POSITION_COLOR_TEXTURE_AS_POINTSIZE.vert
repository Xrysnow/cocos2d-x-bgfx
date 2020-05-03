#pragma once

const char POSITION_COLOR_TEXTURE_AS_POINTSIZE_vert[] =
R"($input a_position, a_color0, a_texcoord0
$output v_color0

uniform float u_alpha;
uniform mat4 u_MVPMatrix;

void main()
{
    gl_Position = mul(u_MVPMatrix, vec4(a_position, 1.0));
    gl_Position.xy = applyVP(gl_Position.xy);
    //TODO: update shaderc
    //gl_PointSize = a_texcoord0.x;
    v_color0 = vec4(a_color0.rgb * a_color0.a * u_alpha, a_color0.a * u_alpha);
}
)";
