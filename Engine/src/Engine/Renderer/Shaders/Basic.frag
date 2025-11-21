#version 430 core

in vec3 v_Normal;
in vec3 v_Color;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform bool u_HasTexture;

void main()
{
    vec3 l_LightDirection = normalize(vec3(0.3, 1.0, 0.5));
    float l_Lighting = max(dot(normalize(v_Normal), l_LightDirection), 0.05);
    vec3 l_BaseColor = v_Color;

    if (u_HasTexture)
    {
        // Sample the atlas and let the texture drive the base color when available.
        l_BaseColor = texture(u_Texture, v_TexCoord).rgb;
    }

    FragColor = vec4(l_BaseColor * l_Lighting, 1.0);
}