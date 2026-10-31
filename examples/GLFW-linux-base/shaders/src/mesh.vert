#version 450
#extension GL_EXT_multiview : require
// Rebuild with:
//   glslc --target-env=vulkan1.0 src/mesh.vert -o mesh.vert.spv
//
// The uniform block matches evan::Frame::UniformBufferObject: one model
// matrix followed by one view/projection pair per eye. The vertex shader
// selects the pair with gl_ViewIndex, which is 0 outside multiview passes
// and the framebuffer layer index inside one.
//
// The per-instance model matrix (evan::GPUVertex instance attributes,
// locations 3 to 6) carries the object transform uploaded by
// Engine::setObjectTransform: the engine always binds the instance buffer and
// addresses the object's slot through firstInstance, so `ubo.model` stays
// identity and the instance matrix carries the pose.

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

layout(location = 3) in vec4 inModel0;
layout(location = 4) in vec4 inModel1;
layout(location = 5) in vec4 inModel2;
layout(location = 6) in vec4 inModel3;

layout(location = 0) out vec3 fragColor;

void main()
{
    ViewUniform view = ubo.views[gl_ViewIndex];
    mat4 instanceModel = mat4(inModel0, inModel1, inModel2, inModel3);

    gl_Position = view.proj * view.view * ubo.model * instanceModel
        * vec4(inPosition, 1.0);
    fragColor   = inColor;
}
