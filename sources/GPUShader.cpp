/*
** ETIB PROJECT, 2025
** evan
** File description:
** GPUShader
*/

#include "evan/GPUShader.hpp"

#include <unordered_set>

namespace
{
	/**
	 * @brief First word of every SPIR-V module.
	 */
	constexpr uint32_t kSpirvMagic = 0x07230203u;

	/**
	 * @brief Index of the first instruction (after the 5 header words).
	 */
	constexpr std::size_t kSpirvHeaderSize = 5;

	/**
	 * @brief Opcode of `OpVariable`.
	 */
	constexpr uint16_t kOpVariable = 59;

	/**
	 * @brief Opcode of `OpDecorate`.
	 */
	constexpr uint16_t kOpDecorate = 71;

	/**
	 * @brief `Decoration` value of a `Location` decoration.
	 */
	constexpr uint32_t kDecorationLocation = 30;

	/**
	 * @brief `StorageClass` value of an `Input` variable.
	 */
	constexpr uint32_t kStorageClassInput = 1;

	/**
	 * @brief First location of the per-instance model matrix.
	 */
	constexpr uint32_t kFirstInstanceLocation = 3;

	/**
	 * @brief Number of `vec4` locations the instance matrix occupies.
	 */
	constexpr uint32_t kInstanceLocationCount = 4;

	/**
	 * @brief Reads the instruction length of the word at @p index.
	 *
	 * @return The number of words of the instruction, 0 when it does not fit
	 * in the module (which means the module is malformed or truncated).
	 */
	uint32_t instructionLength(const std::vector<uint32_t> &code,
							   std::size_t index)
	{
		const uint32_t wordCount = code[index] >> 16;

		if (wordCount == 0 || index + wordCount > code.size()) {
			return 0;
		}
		return wordCount;
	}
}	 // namespace

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

	// Resolved once here: the renderer asks this for every draw, so the SPIR-V
	// input interface must not be walked again per frame.
	_declaresInstanceInputs = declaresInstanceInputs(vertexCode);

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
	destroy();
}

////////////////////
// Public Methods //
////////////////////

void evan::GPUShader::destroy()
{
	this->cleanup();
}

///////////////////////
// Protected Methods //
///////////////////////

void evan::GPUShader::cleanup()
{
	if (_logicalDevice == VK_NULL_HANDLE) {
		return;
	}

	if (_vertexShaderModule != VK_NULL_HANDLE) {
		this->getLogger().info() << "Destroying vertex shader module...";
		vkDestroyShaderModule(_logicalDevice, _vertexShaderModule, nullptr);
		_vertexShaderModule = VK_NULL_HANDLE;
	}

	if (_fragmentShaderModule != VK_NULL_HANDLE) {
		this->getLogger().info() << "Destroying fragment shader module...";
		vkDestroyShaderModule(_logicalDevice, _fragmentShaderModule, nullptr);
		_fragmentShaderModule = VK_NULL_HANDLE;
	}
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

bool evan::GPUShader::declaresInstanceInputs(
	const std::vector<uint32_t> &vertexCode)
{
	if (vertexCode.size() < kSpirvHeaderSize || vertexCode[0] != kSpirvMagic) {
		return false;
	}

	// The decorations and the variables that carry them are separate
	// instructions in any order, so the Input variables are collected first
	// and the locations are matched against them afterwards.
	std::unordered_set<uint32_t> inputIDs;
	for (std::size_t index = kSpirvHeaderSize; index < vertexCode.size();) {
		const uint32_t length = instructionLength(vertexCode, index);

		if (length == 0) {
			return false;
		}
		const uint16_t opcode =
			static_cast<uint16_t>(vertexCode[index] & 0xffffu);
		// OpVariable: [opcode|length][result type][result id][storage class]
		if (opcode == kOpVariable && length >= 4
			&& vertexCode[index + 3] == kStorageClassInput) {
			inputIDs.insert(vertexCode[index + 2]);
		}
		index += length;
	}

	if (inputIDs.empty()) {
		return false;
	}

	// Every location of the matrix has to be declared: a shader exposing only
	// part of it cannot place an instance at its own transform.
	uint32_t declaredLocations = 0;
	for (std::size_t index = kSpirvHeaderSize; index < vertexCode.size();) {
		const uint32_t length = instructionLength(vertexCode, index);

		if (length == 0) {
			return false;
		}
		const uint16_t opcode =
			static_cast<uint16_t>(vertexCode[index] & 0xffffu);
		// OpDecorate: [opcode|length][target id][decoration][literals...]
		if (opcode == kOpDecorate && length >= 4
			&& vertexCode[index + 2] == kDecorationLocation
			&& inputIDs.count(vertexCode[index + 1]) != 0) {
			const uint32_t location = vertexCode[index + 3];

			if (location >= kFirstInstanceLocation
				&& location < kFirstInstanceLocation + kInstanceLocationCount) {
				declaredLocations |= 1u << (location - kFirstInstanceLocation);
			}
		}
		index += length;
	}

	const uint32_t allLocations = (1u << kInstanceLocationCount) - 1u;
	return declaredLocations == allLocations;
}

bool evan::GPUShader::declaresInstanceInputs() const
{
	return _declaresInstanceInputs;
}
