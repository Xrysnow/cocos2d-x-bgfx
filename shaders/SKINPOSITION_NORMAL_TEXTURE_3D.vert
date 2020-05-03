#pragma once

const char SKINPOSITION_NORMAL_TEXTURE_3D_vert[] =
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

//const int SKINNING_JOINT_COUNT = 60;
uniform vec4 u_matrixPalette[60 * 3];
uniform mat4 u_MVMatrix;
uniform mat4 u_PMatrix;
uniform mat3 u_NormalMatrix;

void main()
{
    vec4 position;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;

    vec4 a_blendWeight = a_color0;
    vec4 a_blendIndex = a_color1;
    vec3 a_binormal = a_texcoord1;

    float blendWeight = a_blendWeight[0];

    int matrixIndex = int (a_blendIndex[0]) * 3;
    vec4 matrixPalette1 = mul(u_matrixPalette[matrixIndex], blendWeight);
    vec4 matrixPalette2 = mul(u_matrixPalette[matrixIndex + 1], blendWeight);
    vec4 matrixPalette3 = mul(u_matrixPalette[matrixIndex + 2], blendWeight);


    blendWeight = a_blendWeight[1];
    if (blendWeight > 0.0)
    {
        matrixIndex = int(a_blendIndex[1]) * 3;
        matrixPalette1 += mul(u_matrixPalette[matrixIndex], blendWeight);
        matrixPalette2 += mul(u_matrixPalette[matrixIndex + 1], blendWeight);
        matrixPalette3 += mul(u_matrixPalette[matrixIndex + 2], blendWeight);

        blendWeight = a_blendWeight[2];
        if (blendWeight > 0.0)
        {
            matrixIndex = int(a_blendIndex[2]) * 3;
            matrixPalette1 += mul(u_matrixPalette[matrixIndex], blendWeight);
            matrixPalette2 += mul(u_matrixPalette[matrixIndex + 1], blendWeight);
            matrixPalette3 += mul(u_matrixPalette[matrixIndex + 2], blendWeight);

            blendWeight = a_blendWeight[3];
            if (blendWeight > 0.0)
            {
                matrixIndex = int(a_blendIndex[3]) * 3;
                matrixPalette1 += mul(u_matrixPalette[matrixIndex], blendWeight);
                matrixPalette2 += mul(u_matrixPalette[matrixIndex + 1], blendWeight);
                matrixPalette3 += mul(u_matrixPalette[matrixIndex + 2], blendWeight);
            }
        }
    }

    vec4 p = vec4(a_position, 1.0);
    position.x = dot(p, matrixPalette1);
    position.y = dot(p, matrixPalette2);
    position.z = dot(p, matrixPalette3);
    position.w = p.w;

#if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
    vec4 n = vec4(a_normal, 0.0);
    normal.x = dot(n, matrixPalette1);
    normal.y = dot(n, matrixPalette2);
    normal.z = dot(n, matrixPalette3);
#ifdef USE_NORMAL_MAPPING
    vec4 t = vec4(a_tangent, 0.0);
    tangent.x = dot(t, matrixPalette1);
    tangent.y = dot(t, matrixPalette2);
    tangent.z = dot(t, matrixPalette3);
    vec4 b = vec4(a_binormal, 0.0);
    binormal.x = dot(b, matrixPalette1);
    binormal.y = dot(b, matrixPalette2);
    binormal.z = dot(b, matrixPalette3);
#endif
#endif

    vec4 ePosition = mul(u_MVMatrix, position);

#ifdef USE_NORMAL_MAPPING
    #if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
        vec3 eTangent = normalize(mul(u_NormalMatrix, tangent));
        vec3 eBinormal = normalize(mul(u_NormalMatrix, binormal));
        vec3 eNormal = normalize(mul(u_NormalMatrix, normal));
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
            v_vertexToPointLightDirection[i] = u_PointLightSourcePosition[i].xyz- ePosition.xyz;
        }
    #endif

    #if (MAX_SPOT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_SPOT_LIGHT_NUM; ++i)
        {
            v_vertexToSpotLightDirection[i] = u_SpotLightSourcePosition[i] - ePosition.xyz;
        }
    #endif

    #if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
        v_normal = mul(u_NormalMatrix, vec4(normal, 1.0));
    #endif
#endif

    TextureCoordOut = a_texcoord0;
    TextureCoordOut.y = 1.0 - TextureCoordOut.y;
    gl_Position = mul(u_PMatrix, ePosition);
    gl_Position.xy = applyVP(gl_Position.xy);
}
)";
