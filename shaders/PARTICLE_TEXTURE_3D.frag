#pragma once

const char PARTICLE_TEXTURE_3D_frag[] =
R"($input TextureCoordOut, ColorOut

SAMPLER2D(u_texture, 0);
uniform vec4 u_color;

void main()
{
    gl_FragColor = texture2D(u_texture, TextureCoordOut) * ColorOut * u_color;
}
)";
