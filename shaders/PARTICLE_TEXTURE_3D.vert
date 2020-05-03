#pragma once

const char PARTICLE_TEXTURE_3D_vert[] =
R"($input a_position, a_color0, a_texcoord0
$output TextureCoordOut, ColorOut

uniform mat4 u_PMatrix;

void main()
{
    gl_Position = mul(u_PMatrix, vec4(a_position, 1.0));
    gl_Position.xy = applyVP(gl_Position.xy);
    TextureCoordOut = a_texcoord0;
    TextureCoordOut.y = 1.0 - TextureCoordOut.y;
    ColorOut = a_color0;
}
)";
