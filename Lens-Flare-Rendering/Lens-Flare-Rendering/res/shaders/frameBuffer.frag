
#version 330 core

in vec2 texCoords;
uniform sampler2D screenTexture;

out vec4 fragColor;

uniform vec4 uScale = vec4(0.8f);
uniform vec4 uBias = vec4(0.2f);

// GHOSTS
uniform int uGhosts = 3;
uniform float uGhostDispersal = 0.0001f;
uniform float uGhostWeight = 0.5f;

// HALO
uniform float uHaloWidth = 0.3f;
uniform float uHaloWeight = 10.0f;

// BLURRING
const float offset = 1.0 / 300.0;

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
    
//    float kernel[9] = float[](
//        1.0 / 16, 2.0 / 16, 1.0 / 16,
//        2.0 / 16, 4.0 / 16, 2.0 / 16,
//        1.0 / 16, 2.0 / 16, 1.0 / 16
//    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, texCoords.st + offsets[i]));
    }


    vec2 texcoord = -texCoords + vec2(1.0);
//    vec2 texcoord = texCoords;
    
    // GHOST SAMPLING
    vec2 ghostVec = (vec2(0.5) - texcoord) * uGhostDispersal;
    vec4 result = vec4(0.0);

     for (int i = 0; i < uGhosts; ++i) {
        vec2 offset = fract(texcoord + ghostVec * float(i));

        float weight = length(vec2(0.5) - offset) / length(vec2(0.5));

        weight = pow(1.0 - weight, uGhostWeight);

        result += texture(screenTexture, offset) * weight;
     }

    vec2 haloVec = normalize(ghostVec) * uHaloWidth;
    float weight = length(vec2(0.5) - fract(texcoord + haloVec)) / length(vec2(0.5));
    weight = pow(1.0 - weight, uHaloWeight);
    result += texture(screenTexture, texcoord + haloVec) * weight;
    
    vec3 col = vec3(result);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    
//    fragColor = max(vec4(col,1.0), texture(screenTexture, texCoords) + uBias) * uScale;
        fragColor = max(result, texture(screenTexture, texCoords) + uBias) * uScale;
}
