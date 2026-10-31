#version 450
// Rebuild with:
//   glslc --target-env=vulkan1.0 src/text.frag -o text.frag.spv

layout(binding = 4) uniform sampler2D fontAtlas;

layout(location = 1) in vec2 fragUV;
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
    float glyph = texture(fontAtlas, fragUV).r;
    float alpha = smoothstep(0.08, 0.35, glyph);

    if (alpha < 0.01) {
        discard;
    }

    outColor = vec4(fragColor, alpha);
}
