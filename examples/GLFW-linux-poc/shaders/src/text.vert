#version 450
#extension GL_EXT_multiview : require
// Rebuild with:
//   glslc --target-env=vulkan1.0 src/text.vert -o text.vert.spv
//
// The uniform block matches evan::Frame::UniformBufferObject: one model
// matrix followed by one view/projection pair per eye. The vertex shader
// selects the pair with gl_ViewIndex, which is 0 outside multiview passes
// and the framebuffer layer index inside one.

struct ViewUniform {
    mat4 view;
    mat4 proj;
};

layout(binding = 0, std140) uniform UBO {
    mat4 model;
    ViewUniform views[2];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

void main()
{
    ViewUniform view = ubo.views[gl_ViewIndex];

    fragColor   = inColor;
    fragUV      = inTexCoord;
    gl_Position = view.proj * view.view * ubo.model * vec4(inPosition, 1.0);
}
