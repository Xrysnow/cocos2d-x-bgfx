#pragma once

const char POSITION_TEXTURE_3D_frag[] =
R"($input TextureCoordOut

SAMPLER2D(u_texture, 0);
uniform vec4 u_color;

void main()
{
    gl_FragColor = texture2D(u_texture, TextureCoordOut) * u_color;
}
)";
