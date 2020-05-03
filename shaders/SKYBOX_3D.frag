#pragma once

const char SKYBOX_3D_frag[] =
R"($input v_reflect

SAMPLERCUBE(u_texture, 0);
uniform vec4 u_color;

void main()
{
    gl_FragColor = textureCube(u_texture, v_reflect) * u_color;
}
)";
