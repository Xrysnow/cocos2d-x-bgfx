#pragma once

const char GRAY_SCALE_frag[] =
R"($input v_color0, v_texcoord0

SAMPLER2D(u_texture, 0);

void main()
{
    vec4 c = v_color0 * texture2D(u_texture, v_texcoord0);
    gl_FragColor.rgb = vec3_splat(0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
    gl_FragColor.a = c.a;
}
)";
