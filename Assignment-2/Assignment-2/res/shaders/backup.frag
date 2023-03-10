#version 330 core

out vec4 FragColor;

in vec3 Position;
in vec3 Normal; // this should be normalized
//in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 viewPos;

uniform vec3 refractiveIndexRGB;
uniform float refractiveIndex;
uniform float FresnelPower;
uniform float scale;
uniform float power;

bool reflectIt;

void main()
{
    vec3 I = normalize(Position - viewPos);
    
    vec3 refl = reflect(-I, normalize(Normal));
    vec3 reflectSample = texture(skybox, refl).rgb;
    
    // Refraction with chromatic dispersion
    vec3 refrR = refract(I, normalize(Normal), refractiveIndexRGB.r);
    vec3 refrG = refract(I, normalize(Normal), refractiveIndexRGB.g);
    vec3 refrB = refract(I, normalize(Normal), refractiveIndexRGB.b);

    vec3 refractSample;
    refractSample.r = texture(skybox, refrR).r;
    refractSample.g = texture(skybox, refrG).g;
    refractSample.b = texture(skybox, refrB).b;

    // Reflection Coefficient
    float reflectionCoefficient = max(0, min(1, FresnelPower + scale * pow(1 + dot(I, normalize(Normal)), power) ));
    vec3 result = reflectionCoefficient * reflectSample + ( 1 - reflectionCoefficient) * refractSample;

    FragColor = vec4(result, 1.0);
}
