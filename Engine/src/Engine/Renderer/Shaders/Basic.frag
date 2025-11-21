#version 430 core

in vec3 v_Normal;
in vec2 v_UV;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    vec3 l_LightDirection = normalize(vec3(0.3, 1.0, 0.5));
    float l_Lighting = max(dot(normalize(v_Normal), l_LightDirection), 0.05);
    vec3 l_BaseColor = texture(u_Texture, v_UV).rgb;
    FragColor = vec4(l_BaseColor * l_Lighting, 1.0);
}