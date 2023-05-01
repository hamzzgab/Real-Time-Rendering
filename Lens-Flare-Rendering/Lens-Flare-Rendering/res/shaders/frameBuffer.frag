
#version 330 core

in vec2 texCoords;
uniform sampler2D screenTexture;

out vec4 fragColor;

uniform vec4 uScale;
uniform vec4 uBias;

// GHOSTS
uniform int uGhosts = 3;
uniform float uGhostDispersal = 0.001f;
uniform float uGhostWeight = 0.5f;

// HALO
uniform float uHaloWidth = 0.25f;
uniform float uHaloWeight = 10.0f;


// DISTORTION
uniform float uDistortion;

// BLURRING
uniform bool uBlurIt;
const float offset = 1.0 / 300.0;


vec4 textureDistorted(in sampler2D tex, in vec2 texcoord, in vec2 direction, in vec3 distortion)
{
  return vec4(
     texture(tex, texcoord + direction * distortion.r).r,
     texture(tex, texcoord + direction * distortion.g).g,
     texture(tex, texcoord + direction * distortion.b).b, 1.0f
  );
}

void main()
{
    
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );

    float kernel[9] = float[](
        1.0 / 9, 1.0 / 9, 1.0 / 9,
        1.0 / 9, 1.0 / 9, 1.0 / 9,
        1.0 / 9, 1.0 / 9, 1.0 / 9
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, texCoords.st + offsets[i]));
    }

    vec2 texcoord = -texCoords + vec2(1.0);

    // GHOST SAMPLING
    vec2 ghostVec = (vec2(0.5) - texcoord) * uGhostDispersal;
    vec4 result = vec4(0.0);

     for (int i = 0; i < uGhosts; ++i) {
        vec2 offset = fract(texcoord + ghostVec * float(i));

        float weight = length(vec2(0.5) - offset) / length(vec2(0.5));

        weight = pow(1.0 - weight, uGhostWeight);

        result += texture(screenTexture, offset) * weight;
     }
    
    // HALO
    vec2 haloVec = normalize(ghostVec) * uHaloWidth;
    float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));
    weight = pow(1.0 - weight, uHaloWeight);
    result += texture(screenTexture, texcoord + haloVec) * weight;
    
//    vec2 texelSize = 1.0 / vec2(textureSize(screenTexture, 0));
    vec2 texelSize = 1.0 / vec2(0.25f, 0.25f);
    vec3 distortion = vec3(-texelSize.x * uDistortion, 0.0, texelSize.x * uDistortion);
    vec2 direction = normalize(ghostVec);

    result += textureDistorted(screenTexture, texcoord, direction, distortion);

    vec3 col = vec3(result);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    if (uBlurIt)
    {
        fragColor = max(vec4(col,1.0), texture(screenTexture, texCoords) + uBias) * uScale;
    }else
    {
        fragColor = max(result, texture(screenTexture, texCoords) + uBias) * uScale;
    }
}
