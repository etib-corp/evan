/*
** ETIB PROJECT, 2025
** evan
** File description:
** Shader
*/

#include "Shader.hpp"

evan::Shader::Shader(const std::vector<uint32_t> &vertexCode,
					 const std::vector<uint32_t> &fragmentCode, VkDevice device)
{
	VkShaderModuleCreateInfo createInfo {};

	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = vertexCode.size() * sizeof(uint32_t);
	createInfo.pCode	= vertexCode.data();

	_vertexCode	  = vertexCode;
	_fragmentCode = fragmentCode;
	_logicalDevice = device;

	if (vkCreateShaderModule(device, &createInfo, nullptr, &_vertexShaderModule)
		!= VK_SUCCESS) {
		// throw an error if the shader module creation fails
		return;
	}

	createInfo.codeSize = fragmentCode.size() * sizeof(uint32_t);
	createInfo.pCode	= fragmentCode.data();

	if (vkCreateShaderModule(device, &createInfo, nullptr,
							 &_fragmentShaderModule)
		!= VK_SUCCESS) {
		// throw an error if the shader module creation fails
		return;
	}
}

evan::Shader::~Shader()
{
	vkDestroyShaderModule(_logicalDevice, _vertexShaderModule, nullptr);
	vkDestroyShaderModule(_logicalDevice, _fragmentShaderModule, nullptr);
}

VkShaderModule evan::Shader::getVertexShaderModule() const
{
	return _vertexShaderModule;
}

VkShaderModule evan::Shader::getFragmentShaderModule() const
{
	return _fragmentShaderModule;
}
