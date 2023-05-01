# Research Implementation

## About

This is the Research Implementation for Real-Time Lens Flare Rendering (CS7GV3) at Trinity College Dublin.

## Implementation
The Lens Flare Generator is used in the framebuffer.frag, following is the code for it.
```
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
vec4 textureDistorted(in sampler2D tex, in vec2 texcoord, in vec2 direction, in vec3 distortion)
{
  return vec4(
     texture(tex, texcoord + direction * distortion.r).r,
     texture(tex, texcoord + direction * distortion.g).g,
     texture(tex, texcoord + direction * distortion.b).b, 1.0f
  );
}


// BLURRING
uniform bool uBlurIt;
const float offset = 1.0 / 300.0;

void main()
{
    // GAUSSIAN BLURRING
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
    
    // FLIPPING TEXCOORDS
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
    
    // CHROMATIC DISTORTION
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
```

## Demonstration

The video demonstration for the assignment can be found on [YouTube](https://youtu.be/BhXuaUNSYYs)
<p align='center'>
  <a href="http://www.youtube.com/watch?feature=player_embedded&v=BhXuaUNSYYs" target="_blank">
      <img src="http://img.youtube.com/vi/BhXuaUNSYYs/0.jpg" alt="Research Implementation Real Time Rendering" width="240" height="180" border="10" />
  </a>
<p>

## References

[1] M. Hullin, E. Eisemann, H.-P. Seidel, and S. Lee, “Physically-based real-time lens flare rendering,” in ACM SIGGRAPH 2011 Papers, ser. SIGGRAPH ’11. New York, NY, USA: Association for Computing Machinery, 2011. [Online]. Available: https://doi-org.elib.tcd.ie/10.1145/1964921.1965003
  
[2] “Pseudo lens flare.” [Online]. Available: http://john-chapman-graphics.blogspot.com/2013/02/pseudo-lens-flare.html
  
[3] “Framebuffers.” [Online]. Available: https://learnopengl.com/Advanced-OpenGL/Framebuffers
  
[4] Ocornut, “Ocornut/imgui: Dear imgui: Bloat-free graphical user interface for C++ with minimal dependencies.” [Online]. Available: https://github.com/ocornut/imgui
