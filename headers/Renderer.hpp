/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#pragma once

#include "ADeviceBackend.hpp"

#include "Frame.hpp"
#include "Shader.hpp"
#include "Vertex.hpp"

#include <fstream>
#include <algorithm>

/*
 * @brief Maximum number of frames in flight.
 *
 * This constant defines the maximum number of frames that can be in flight
 * at any given time. It is used to manage synchronization and resource
 * allocation for rendering operations in Vulkan.
 *
 */
const int MAX_FRAMES_IN_FLIGHT = 2;

namespace evan {
    class Renderer {
        public:
        Renderer();
        ~Renderer();

        void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples);

        void createDescriptorPool(VkDevice device, uint32_t materialCount);

        protected:
            VkPipeline _pipeline;
            VkPipelineLayout _pipelineLayout;
            std::vector<Frame> _frames;
            uint32_t _currentFrameIndex;
            VkDescriptorSetLayout _descriptorSetLayout;
            VkDescriptorPool _descriptorPool;

        private:
            std::vector<uint32_t> readFile(const std::string &filename);

    };
}
