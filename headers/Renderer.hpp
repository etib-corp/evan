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
