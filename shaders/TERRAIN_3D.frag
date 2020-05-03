#pragma once

const char TERRAIN_3D_frag[] =
R"($input v_texcoord0, v_normal

SAMPLER2D(u_alphaMap, 0);
SAMPLER2D(u_texture0, 1);
SAMPLER2D(u_texture1, 2);
SAMPLER2D(u_texture2, 3);
SAMPLER2D(u_texture3, 4);
SAMPLER2D(u_lightMap, 5);
uniform vec3 u_color;
uniform float u_has_alpha;
uniform float u_has_light_map;
uniform float u_detailSize[4];
uniform vec3 u_lightDir;

void main()
{
    vec4 lightColor;
    if(u_has_light_map <= 0.0)
    {
        lightColor = vec4(1.0,1.0,1.0,1.0);
    }
    else
    {
        lightColor = texture2D(u_lightMap, v_texcoord0);
    }
    float lightFactor = dot(-u_lightDir, v_normal);
    if(u_has_alpha <= 0.0)
    {
        gl_FragColor = texture2D(u_texture0, v_texcoord0) * lightColor * lightFactor;
    }
    else
    {
        vec4 blendFactor =texture2D(u_alphaMap,v_texcoord0);
        vec4 color =
            texture2D(u_texture0, v_texcoord0 * u_detailSize[0]) * blendFactor.r
            + texture2D(u_texture1, v_texcoord0 * u_detailSize[1]) * blendFactor.g
            + texture2D(u_texture2, v_texcoord0 * u_detailSize[2]) * blendFactor.b
            + texture2D(u_texture3, v_texcoord0 * u_detailSize[3]) * (1.0 - blendFactor.a);
        gl_FragColor = vec4(color.rgb * lightColor.rgb * lightFactor, 1.0);
    }
}
)";
