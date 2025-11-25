#version 330 core
in vec3 vColor;
in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D tex;
uniform int useTexture;

void main() 
{
    if (useTexture == 1)
        FragColor = texture(tex, vTexCoord);
    else
        FragColor = vec4(vColor, 1.0);
}
