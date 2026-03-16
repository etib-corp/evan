/*
** ETIB PROJECT, 2026
** evan
** File description:
** GPUMesh
*/

#include "GPUMesh.hpp"

evan::GPUMesh::GPUMesh(const DeviceContext &deviceContext, std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t materialID)
{
    auto deviceBackend = deviceContext.getDeviceBackend();
	_indexCount = indices.size();
	_materialID = materialID;

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	if (bufferSize == 0) {
		return;
	}

	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._logicalDevice	 = deviceBackend->_device,
		._physicalDevice = deviceBackend->_physicalDevice,
		._size			 = bufferSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties	 = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	void *data;
	vkMapMemory(deviceBackend->_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(deviceBackend->_device, stagingBufferMemory);

	ADeviceBackend::CreateBufferProperties vertexBufferProperties = {
		._logicalDevice	 = deviceBackend->_device,
		._physicalDevice = deviceBackend->_physicalDevice,
		._size			 = bufferSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _vertexBuffer,
		._bufferMemory = _vertexBufferMemory
	};
	deviceBackend->createBuffer(vertexBufferProperties);

	ADeviceBackend::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = deviceBackend->_device,
		._commandPool	= deviceContext.getCommandPool(),
		._graphicsQueue = deviceContext.getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _vertexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	vkDestroyBuffer(deviceBackend->_device, stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend->_device, stagingBufferMemory, nullptr);

    this->createIndexBuffer(deviceContext, indices);
}

evan::GPUMesh::~GPUMesh()
{
}

void evan::GPUMesh::destroy(VkDevice device)
{
	vkDestroyBuffer(device, _vertexBuffer, nullptr);
	vkFreeMemory(device, _vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, _indexBuffer, nullptr);
	vkFreeMemory(device, _indexBufferMemory, nullptr);
}

void evan::GPUMesh::createIndexBuffer(const DeviceContext &deviceContext, std::vector<uint32_t> indices)
{
	auto deviceBackend = deviceContext.getDeviceBackend();

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	if (bufferSize == 0) {
		return;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._logicalDevice	 = deviceBackend->_device,
		._physicalDevice = deviceBackend->_physicalDevice,
		._size			 = bufferSize,
		._usage			 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties	 = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};
	deviceBackend->createBuffer(stagingBufferProperties);

	void *data;
	vkMapMemory(deviceBackend->_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(deviceBackend->_device, stagingBufferMemory);

	ADeviceBackend::CreateBufferProperties indexBufferProperties = {
		._logicalDevice	 = deviceBackend->_device,
		._physicalDevice = deviceBackend->_physicalDevice,
		._size			 = bufferSize,
		._usage =
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _indexBuffer,
		._bufferMemory = _indexBufferMemory
	};
	deviceBackend->createBuffer(indexBufferProperties);

	ADeviceBackend::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = deviceBackend->_device,
		._commandPool	= deviceContext.getCommandPool(),
		._graphicsQueue = deviceContext.getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _indexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	vkDestroyBuffer(deviceBackend->_device, stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend->_device, stagingBufferMemory, nullptr);
}
