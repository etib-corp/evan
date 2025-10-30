/*
** ETIB PROJECT, 2025
** evan
** File description:
** Shader
*/

#include "Shader.hpp"

evan::Shader::Shader(const std::vector<uint32_t> &vertexCode, const std::vector<uint32_t> &fragmentCode, VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = vertexCode.size() * sizeof(uint32_t);
    createInfo.pCode = vertexCode.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &_vertexShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex shader module!");
    }

    createInfo.codeSize = fragmentCode.size() * sizeof(uint32_t);
    createInfo.pCode = fragmentCode.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &_fragmentShaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fragment shader module!");
    }
 
}

evan::Shader::~Shader()
{
}
