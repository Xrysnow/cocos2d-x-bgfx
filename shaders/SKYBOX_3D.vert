#pragma once

const char SKYBOX_3D_vert[] =
R"($input a_position, a_color0, a_texcoord0
$output v_reflect

uniform mat4 u_cameraRot;

void main()
{
    vec4 reflect = mul(u_cameraRot, vec4(a_position, 1.0));
    v_reflect = reflect.xyz;
    gl_Position = vec4(a_position.xy, 1.0 , 1.0);
    gl_Position.xy = applyVP(gl_Position.xy);
}
)";
