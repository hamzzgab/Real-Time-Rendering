#version 330 core
struct Light
{
    vec3 direction;
};

in vec3 Normal;

out vec4 color;

uniform vec3 viewPos;
uniform Light light;

void main()
{
    float intensity;
    intensity = dot(-light.direction, normalize(Normal));

    if (intensity > 0.95)
        color = vec4(0.3, 1.0, 0.3, 1.0);
    else if (intensity > 0.75)
        color = vec4(0.2, 0.8, 0.2, 1.0);
    else if (intensity > 0.55)
        color = vec4(0.1, 0.6, 0.1, 1.0);
    else if (intensity > 0.35)
        color = vec4(0.0, 0.4, 0.0, 1.0);
    else if (intensity > 0.15)
        color = vec4(0.0, 0.2, 0.0, 1.0);
    else
        color = vec4(0.0, 0.0, 0.0, 1.0);
}

