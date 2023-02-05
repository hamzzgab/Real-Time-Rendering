#version 330 core

out vec4 FragColor;

in vec3 Position;
in vec3 Normal; // this should be normalized
//in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 viewPos;

uniform vec3 refractiveIndexRGB;
uniform float refractiveIndex;
uniform float bias;
uniform float scale;
uniform float power;

bool reflectIt;

void main()
{
    vec3 I = normalize(Position - viewPos);
    
    vec3 refl = reflect(-I, normalize(Normal));
    vec3 reflectSample = texture(skybox, refl).rgb;
    
    vec3 result = reflectSample;
    
    FragColor = vec4(result, 1.0);

}
