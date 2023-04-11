#version 330 core

out vec4 FragColor;

in vec3 Position;
in vec3 Normal;

uniform samplerCube skybox;
uniform vec3 viewPos;

void main()
{
    vec3 I = normalize(Position - viewPos);
    
    vec3 refl = reflect(-I, normalize(Normal));
    vec3 reflectSample = texture(skybox, refl).rgb;
    
    vec3 result = reflectSample;
    
    FragColor = vec4(result, 1.0);

}
