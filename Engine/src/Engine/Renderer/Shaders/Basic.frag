#version 430 core

in vec3 v_Normal;
in vec3 v_Color;
in vec2 v_TexCoord;

out vec4 FragColor;

layout(std140, binding = 0) uniform PerFrame
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_LightDirection; // xyz is the direction, w is unused for now
    vec4 u_LightColor;     // xyz is the color, w is unused for now
    float u_AmbientStrength;
};

uniform sampler2D u_Texture;
uniform bool u_HasTexture;

void main()
{
    vec3 l_BaseColor = v_Color;

    if (u_HasTexture)
    {
        // Sample the atlas and let the texture drive the base color when available.
        l_BaseColor = texture(u_Texture, v_TexCoord).rgb;
    }

    vec3 l_Normal = normalize(v_Normal);
    vec3 l_LightDir = normalize(u_LightDirection.xyz);
    float l_Diffuse = max(dot(l_Normal, l_LightDir), 0.0);
    vec3 l_Ambient = u_LightColor.rgb * u_AmbientStrength;
    vec3 l_Lighting = l_Ambient + (u_LightColor.rgb * l_Diffuse);

    FragColor = vec4(l_BaseColor * l_Lighting, 1.0);
}