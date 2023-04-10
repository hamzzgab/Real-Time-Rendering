
#version 330 core

in vec2 texCoords;
uniform sampler2D screenTexture;

uniform vec2 lightPos = vec2(0.0, 1.0);  // the position of the light source
uniform float intensity;  // the intensity of the effect

uniform float val;

out vec4 fragColor;

/*
uniform float exposure = 0.3f;
uniform float bloomThreshold = 0.9f;
uniform float bloomIntensity = 0.6f;
uniform vec2 blurSize = vec2(0.5f, 0.5f);
uniform int blurPasses = 5;

void main()
{
    vec4 color = texture(screenTexture, texCoords);
    
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    
    vec4 bloomColor = vec4(0.0f);
    
    if (brightness > bloomThreshold)
    {
        bloomColor = color * (brightness - bloomThreshold) * bloomIntensity;
    }
    
    vec4 blur = vec4(0.0f);
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    
    for (int i = 0; i < blurPasses; i++) {
            for (float x = -4.0; x <= 4.0; x += 1.0) {
                for (float y = -4.0; y <= 4.0; y += 1.0) {
                    vec2 offset = vec2(x, y) * blurSize;
                    blur += texture(screenTexture, texCoords + offset * texelSize);
                }
            }
            bloomColor += blur / 81.0;
            blur = vec4(0.0);
        }
    
    fragColor = color + bloomColor * exposure;
}

 */
/*

void main() {
    vec4 color = texture(screenTexture, texCoords);
    
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));

    // calculate lens flare effect
    vec2 flareCoords = texCoords * vec2(2.0, 2.0) - vec2(1.0, 1.0);
    float flare = pow(1.0 - length(flareCoords), 8.0);
    vec4 flareColor = vec4(1.0, 1.0, 0.5, 1.0) * flare * brightness * val;

    fragColor = color + flareColor;
}
 */

uniform vec4 uScale = vec4(1.0f);
uniform vec4 uBias = vec4(0.25f);

uniform int uGhosts = 2; // number of ghost samples
uniform float uGhostDispersal = 0.0001f; // dispersion factor
float uHaloWidth = 0.25f;

vec3 textureDistorted(
      in sampler2D tex,
      in vec2 texcoord,
      in vec2 direction, // direction of distortion
      in vec3 distortion // per-channel distortion factor
   )
{
      return vec3(
         texture(tex, texcoord + direction * distortion.r).r,
         texture(tex, texcoord + direction * distortion.g).g,
         texture(tex, texcoord + direction * distortion.b).b
      );
}

void main()
{
  
    vec2 texcoord = -texCoords + vec2(1.0);
    
    vec2 texelSize = 1.0 / vec2(textureSize(screenTexture, 0));

    vec2 ghostVec = (vec2(0.5) - texcoord) * uGhostDispersal;

    // sample ghosts:
    vec4 result = vec4(0.0);
    
     for (int i = 0; i < uGhosts; ++i) {
        vec2 offset = fract(texcoord + ghostVec * float(i));
        
        float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
         
        weight = pow(1.0 - weight, 10.0);
    
        result += texture(screenTexture, offset) * weight;
     }
    
//        result *= texture(uLensCOLOR, length(vec2(0.5) - texcoord) / length(vec2(0.5)));
    
    vec2 haloVec = normalize(ghostVec) * uHaloWidth;
    float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));
    weight = pow(1.0 - weight, 10.0);
    result += texture(screenTexture, texcoord + haloVec) * weight;
        
//    float uDistortion = 0.2f;
//    vec2 texelSize = 1.0 / vec2(textureSize(screenTexture, 0));
//    vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);
//    vec3 direction = normalize(vec2(0.0f, 1.0f));
//    result += textureDistorted(screenTexture, texcoord, direction, distortion);

    fragColor = max(result, texture(screenTexture, texCoords) + uBias) * uScale;
      
}
