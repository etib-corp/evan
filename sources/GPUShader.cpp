/*
** ETIB PROJECT, 2025
** evan
** File description:
** GPUShader
*/

#include "evan/GPUShader.hpp"

evan::GPUShader::GPUShader(VkDevice device,
						   const utility::graphic::Shader &shader)
{
	this->getLogger().info() << "Initializing GPUShader...";

	VkShaderModuleCreateInfo createInfo {};

	auto vertexCode	  = shader.getVertexCode();
	auto fragmentCode = shader.getFragmentCode();

	this->getLogger().info()
		<< "Vertex shader code size: " << vertexCode.size() * sizeof(uint32_t)
		<< " bytes";
	this->getLogger().info()
		<< "Fragment shader code size: "
		<< fragmentCode.size() * sizeof(uint32_t) << " bytes";

	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = vertexCode.size() * sizeof(uint32_t);
	createInfo.pCode	= vertexCode.data();

	_vertexCode	   = vertexCode;
	_fragmentCode  = fragmentCode;
	_logicalDevice = device;

	if (vkCreateShaderModule(device, &createInfo, nullptr, &_vertexShaderModule)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create vertex shader module.";
		return;
	}

	createInfo.codeSize = fragmentCode.size() * sizeof(uint32_t);
	createInfo.pCode	= fragmentCode.data();

	if (vkCreateShaderModule(device, &createInfo, nullptr,
							 &_fragmentShaderModule)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create fragment shader module.";
		return;
	}
}

evan::GPUShader::~GPUShader()
{
	this->getLogger().info() << "Destroying GPUShader...";
}

////////////////////
// Public Methods //
////////////////////

void evan::GPUShader::destroy()
{
	this->getLogger().info() << "Destroying GPUShader modules...";
	vkDestroyShaderModule(_logicalDevice, _vertexShaderModule, nullptr);

	this->getLogger().info() << "Destroying fragment shader module...";
	vkDestroyShaderModule(_logicalDevice, _fragmentShaderModule, nullptr);
}

/////////////
// Getters //
/////////////

VkShaderModule evan::GPUShader::getVertexShaderModule() const
{
	return _vertexShaderModule;
}

VkShaderModule evan::GPUShader::getFragmentShaderModule() const
{
	return _fragmentShaderModule;
}
