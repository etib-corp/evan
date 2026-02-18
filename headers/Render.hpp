/*
** ETIB PROJECT, 2026
** evan
** File description:
** Render
*/

#pragma once

#include "EvanPlatform.hpp"
#include "Shader.hpp"

#include <vector>

namespace evan
{
    /**
     * @class Render
     * @brief Represents the rendering system of the engine, responsible for managing rendering operations and resources.
     *
     * The Render class encapsulates the functionality related to rendering in the engine. It is responsible for managing rendering operations, resources, and interactions with the graphics API (such as Vulkan). This class serves as a central point for handling all rendering-related tasks in the engine.
     *
     */
    class Render {
        public:
            Render();
            ~Render();

        protected:
            VkPipeline _pipeline;    /// The graphics pipeline, which defines the configuration and state for rendering operations, including shader stages, fixed-function state, and pipeline layout.
            VkPipelineLayout _pipelineLayout;    /// The pipeline layout, which defines the interface between shader stages and the resources used by those stages, such as descriptor sets and push constants.
            uint32_t _currentFrame;    /// The index of the current frame being rendered, used for synchronizing rendering operations and managing resources across multiple frames.
            std::vector<Shader> _shaders;    /// A vector of Shader objects, which represent the shaders used in the rendering pipeline. Each Shader object encapsulates the vertex and fragment shader modules, as well as the shader code.
            // ONLY FOR GLFW
            VkDescriptorSetLayout _descriptorSetLayout;    /// The descriptor set layout, which defines the structure of descriptor sets used for binding resources to shader stages in the graphics pipeline. It specifies the types and counts of resources that can be bound to the pipeline, such as uniform buffers, storage buffers, and samplers.
            VkDescriptorPool _descriptorPool;    /// The descriptor pool, which is used to allocate descriptor sets for binding resources to shader stages in the graphics pipeline. It manages the memory for descriptor sets and allows for efficient allocation and deallocation of descriptor sets as needed during rendering operations.
        private:
    };
} // namespace evan
