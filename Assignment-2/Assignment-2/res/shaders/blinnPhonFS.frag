#version 330 core
struct Material
{
    sampler2D diffuse;
    sampler2D ambient;
    sampler2D specular;

};

struct Light
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool blinn;
uniform float shininess;

// [3] OpenGL. (2014, June 1). In Learn OpenGL. https://learnopengl.com/Getting-started/OpenGL

void main()
{
    // AMBIENT
    vec3 ambient = light.ambient * vec3(texture(material.ambient, TexCoords));
    
    // DIFFUSE
    vec3 norm = normalize(Normal);
    vec3 lightDir = light.direction - FragPos;
    float distance = length(lightDir);
    lightDir = normalize(-light.direction);
    
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords)) / distance * distance;
    
    // SPECULAR
    vec3 viewDir = normalize(viewPos - FragPos);
    
    float spec = 0.0f;
    //BLINN
    if (blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
    }else{
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    }

    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)) / distance * distance;
    
    color = vec4(ambient + diffuse + specular, 1.0f);
//    color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
