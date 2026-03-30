#version 430 core
out vec4 FragColor;

struct Camera {
    vec3 pos;
    vec3 lookDir;
};

uniform Camera camera;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}