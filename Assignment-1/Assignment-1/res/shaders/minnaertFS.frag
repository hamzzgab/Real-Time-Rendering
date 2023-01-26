#version 330 core
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 direction;
    vec3 color;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform float darkenCoefficient;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    
    
    // AMBIENT - ka
//    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 ambient = material.ambient;
    
    // DIFFUSE - kd
    vec3 norm = normalize(Normal);
    vec3 lightDir = light.direction - FragPos;
    float distance = length(lightDir);
    lightDir = normalize(-lightDir);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // minnaert
    float nDotL = dot(norm, lightDir);
    float nDotE = dot(norm, viewDir);
    
    float intensity = max(clamp(nDotL, 0.0, 1.0) * pow(nDotL * nDotE, darkenCoefficient) / distance, 0);

    vec3 result = ambient + intensity * material.diffuse * light.color;

    color = vec4(result, 1.0);
}
