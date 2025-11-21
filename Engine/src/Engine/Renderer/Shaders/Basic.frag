#version 430 core

in vec3 v_Normal;
in vec2 v_UV;

out vec4 FragColor;

void main()
{
    vec3 l_LightDirection = normalize(vec3(0.3, 1.0, 0.5));
    float l_Lighting = max(dot(normalize(v_Normal), l_LightDirection), 0.05);
    vec3 l_BaseColor = vec3(0.6 + 0.4 * v_UV.x, 0.7, 0.6 + 0.4 * v_UV.y);
    FragColor = vec4(l_BaseColor * l_Lighting, 1.0);
}