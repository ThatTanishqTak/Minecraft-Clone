#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;
layout(location = 3) in vec2 a_UV;

layout(std140, binding = 0) uniform PerFrame
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_LightDirection; // xyz is the direction, w is unused for now
    vec4 u_LightColor;     // xyz is the color, w is unused for now
    float u_AmbientStrength;
};

uniform mat4 u_Model;

out vec3 v_Normal;
out vec3 v_Color;
out vec2 v_TexCoord;

void main()
{
    mat3 l_NormalMatrix = mat3(transpose(inverse(u_Model)));
    v_Normal = normalize(l_NormalMatrix * a_Normal);
    v_Color = a_Color;
    v_TexCoord = a_UV;

    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}