#version 330 core

out vec4 FragColor;

in vec3 fFragPos;
in vec3 fNormal; // this should be normalized
//in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 fCameraPosition;

uniform float FresnelPower = 1.0f;
uniform vec3 refractiveIndexRGB;


void main()
{
   vec3 I = normalize(fFragPos - fCameraPosition);

   vec3 nNormal = normalize(fNormal);
   float F = ((1.0 - refractiveIndexRGB.g) * (1.0 - refractiveIndexRGB.g)) / ((1.0 + refractiveIndexRGB.g) * (1.0 + refractiveIndexRGB.g));

   float Ratio = F + (1.0 - F) * pow((1.0 - dot(-I, nNormal)), FresnelPower);

   vec3 RefractR = refract(I, nNormal, refractiveIndexRGB.r);
   vec3 RefractG = refract(I, nNormal, refractiveIndexRGB.g);
   vec3 RefractB = refract(I, nNormal, refractiveIndexRGB.b);

   vec3 Reflect = reflect(I, nNormal);


   vec3 refractColor;
   refractColor.r = texture(skybox, RefractR).r;
   refractColor.g = texture(skybox, RefractG).g;
   refractColor.b = texture(skybox, RefractB).b;

   vec3 reflectColor = texture(skybox, Reflect).rgb;

  vec3 result = mix(reflectColor, refractColor, Ratio);

   FragColor = vec4(result, 1.0);
//    FragColor = vec4(vec3(FresnelPower), 1.0);
}
