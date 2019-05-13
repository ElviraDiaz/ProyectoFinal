#version 330 core

in vec3 customColor;
out vec4 FragColor;


void main()
{
    FragColor = vec4(customColor, 1.0f);
}