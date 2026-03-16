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

namespace evan {
    class Renderer {
        public:
        Renderer(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples);
        ~Renderer();

        void destroy(VkDevice device);

        void createDescriptorSetLayout(VkDevice device);

        void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits msaaSamples);

        void createDescriptorPool(VkDevice device, uint32_t materialCount);

        void createFrame(VkCommandPool commandPool, const ADeviceBackend &deviceBackend);

        VkDescriptorPool getDescriptorPool() const;
        const std::vector<VkBuffer>& getUniformBuffers() const;
        VkDescriptorSetLayout getDescriptorSetLayout() const;

        protected:
            VkPipeline _pipeline;
            VkPipelineLayout _pipelineLayout;
            std::vector<Frame> _frames;
            uint32_t _currentFrameIndex;
            VkDescriptorSetLayout _descriptorSetLayout;
            VkDescriptorPool _descriptorPool;

    };
}
