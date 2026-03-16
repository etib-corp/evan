/*
** ETIB PROJECT, 2026
** evan
** File description:
** Frame
*/

#pragma once

#include "EvanPlatform.hpp"

#include "ADeviceBackend.hpp"

namespace evan {
    class Frame {
        public:
        /**
         * @brief Uniform buffer object structure.
         *
         * This structure represents the data that will be passed to the
         * vertex shader as a uniform buffer. It contains the model, view,
         * and projection matrices used for rendering.
         *
         */
        struct UniformBufferObject {
            glm::mat4 model;	// Model matrix
            glm::mat4 view;		// View matrix
            glm::mat4 proj;		// Projection matrix
        };

        Frame(VkCommandPool commandPool, const ADeviceBackend &deviceBackend);
        ~Frame();

        void destroy(VkDevice device);

        void createCommandBuffer(VkDevice device, VkCommandPool commandPool);
        void createSyncObjects(VkDevice device);
        void createUniformBuffer(const ADeviceBackend &deviceBackend);

        VkBuffer getUniformBuffer() const;

        void resetCommandBuffer();

        VkFence _inFlight;
        VkSemaphore _image;
        VkCommandBuffer _commandBuffer;

        protected:
        VkSemaphore _render;
        VkBuffer _uniformBuffer;
        VkDeviceMemory _uniformBufferMemory;
        void * _uniformBufferMapped;
    };
}
