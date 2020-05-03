#pragma once

const char POSITION_NORMAL_TEXTURE_3D_vert[] =
R"($input a_position, a_color0, a_color1, a_texcoord0, a_normal, a_tangent, a_texcoord1
$output TextureCoordOut, v_dirLightDirection, v_vertexToPointLightDirection, v_vertexToSpotLightDirection, v_spotLightDirection, v_normal

#ifdef USE_NORMAL_MAPPING
#if (MAX_DIRECTIONAL_LIGHT_NUM > 0)
uniform vec3 u_DirLightSourceDirection[MAX_DIRECTIONAL_LIGHT_NUM];
#endif
#endif
#if (MAX_POINT_LIGHT_NUM > 0)
uniform vec3 u_PointLightSourcePosition[MAX_POINT_LIGHT_NUM];
#endif
#if (MAX_SPOT_LIGHT_NUM > 0)
uniform vec3 u_SpotLightSourcePosition[MAX_SPOT_LIGHT_NUM];
#ifdef USE_NORMAL_MAPPING
uniform vec3 u_SpotLightSourceDirection[MAX_SPOT_LIGHT_NUM];
#endif
#endif

uniform mat4 u_MVMatrix;
uniform mat4 u_PMatrix;
uniform mat3 u_NormalMatrix;

void main()
{
    // vec4 a_blendWeight = a_color0;
    // vec4 a_blendIndex = a_color1;
    vec3 a_binormal = a_texcoord1;

    vec4 ePosition = mul(u_MVMatrix, vec4(a_position, 1.0));
#ifdef USE_NORMAL_MAPPING
    #if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
        vec3 eTangent = normalize(mul(u_NormalMatrix, a_tangent));
        vec3 eBinormal = normalize(mul(u_NormalMatrix, vec4(a_binormal, 1.0)));
        vec3 eNormal = normalize(mul(u_NormalMatrix, a_normal));
    #endif
    #if (MAX_DIRECTIONAL_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_DIRECTIONAL_LIGHT_NUM; ++i)
        {
            v_dirLightDirection[i].x = dot(eTangent, u_DirLightSourceDirection[i]);
            v_dirLightDirection[i].y = dot(eBinormal, u_DirLightSourceDirection[i]);
            v_dirLightDirection[i].z = dot(eNormal, u_DirLightSourceDirection[i]);
        }
    #endif

    #if (MAX_POINT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_POINT_LIGHT_NUM; ++i)
        {
            vec3 pointLightDir = u_PointLightSourcePosition[i].xyz - ePosition.xyz;
            v_vertexToPointLightDirection[i].x = dot(eTangent, pointLightDir);
            v_vertexToPointLightDirection[i].y = dot(eBinormal, pointLightDir);
            v_vertexToPointLightDirection[i].z = dot(eNormal, pointLightDir);
        }
    #endif

    #if (MAX_SPOT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_SPOT_LIGHT_NUM; ++i)
        {
            vec3 spotLightDir = u_SpotLightSourcePosition[i] - ePosition.xyz;
            v_vertexToSpotLightDirection[i].x = dot(eTangent, spotLightDir);
            v_vertexToSpotLightDirection[i].y = dot(eBinormal, spotLightDir);
            v_vertexToSpotLightDirection[i].z = dot(eNormal, spotLightDir);

            v_spotLightDirection[i].x = dot(eTangent, u_SpotLightSourceDirection[i]);
            v_spotLightDirection[i].y = dot(eBinormal, u_SpotLightSourceDirection[i]);
            v_spotLightDirection[i].z = dot(eNormal, u_SpotLightSourceDirection[i]);
        }
    #endif
#else
    #if (MAX_POINT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_POINT_LIGHT_NUM; ++i)
        {
            v_vertexToPointLightDirection[i] = u_PointLightSourcePosition[i].xyz - ePosition.xyz;
        }
    #endif

    #if (MAX_SPOT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_SPOT_LIGHT_NUM; ++i)
        {
            v_vertexToSpotLightDirection[i] = u_SpotLightSourcePosition[i] - ePosition.xyz;
        }
    #endif

    #if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
        v_normal = mul(u_NormalMatrix, vec4(a_normal, 1.0));
    #endif
#endif

    TextureCoordOut = a_texcoord0;
    TextureCoordOut.y = 1.0 - TextureCoordOut.y;
    gl_Position = mul(u_PMatrix, ePosition);
    gl_Position.xy = applyVP(gl_Position.xy);
}
)";
