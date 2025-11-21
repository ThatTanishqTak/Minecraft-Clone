#version 430 core

in vec3 v_Normal;
in vec3 v_Color;

out vec4 FragColor;

void main()
{
    vec3 l_LightDirection = normalize(vec3(0.3, 1.0, 0.5));
    float l_Lighting = max(dot(normalize(v_Normal), l_LightDirection), 0.05);
    FragColor = vec4(v_Color * l_Lighting, 1.0);
}