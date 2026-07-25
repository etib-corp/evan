/*
** ETIB PROJECT, 2026
** evan
** File description:
** GPUMesh
*/

#include "evan/GPUMesh.hpp"

evan::GPUMesh::GPUMesh(std::shared_ptr<DeviceContext> deviceContext,
					   std::vector<GPUVertex> vertices,
					   std::vector<uint32_t> indices, uint32_t materialID)
{
	this->getLogger().info()
		<< "Initializing GPUMesh with material ID: " << materialID << "...";

	auto deviceBackend = deviceContext->getDeviceBackend();
	_indexCount		   = indices.size();
	_materialID		   = materialID;

	this->getLogger().info()
		<< "GPUMesh set up with " << vertices.size() << " vertices and "
		<< indices.size() << " indices. Creating vertex buffer...";

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	this->getLogger().info() << "Vertex buffer size: " << bufferSize;
	if (bufferSize == 0) {
		this->getLogger().warning() << "Vertex buffer size is zero.";
		return;
	}

	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._size		 = bufferSize,
		._usage		 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	this->getLogger().info() << "Creating staging buffer for vertex data...";

	deviceBackend->createBuffer(stagingBufferProperties);

	this->getLogger().info()
		<< "Mapping staging buffer memory and copying vertex data...";

	void *data = nullptr;
	vkMapMemory(deviceBackend->_device, stagingBufferMemory, 0, bufferSize, 0,
				&data);
	memcpy(data, vertices.data(), (size_t)bufferSize);

	this->getLogger().info() << "Unmapping staging buffer memory...";

	vkUnmapMemory(deviceBackend->_device, stagingBufferMemory);

	ADeviceBackend::CreateBufferProperties vertexBufferProperties = {
		._size	= bufferSize,
		._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _vertexBuffer,
		._bufferMemory = _vertexBufferMemory
	};
	deviceBackend->createBuffer(vertexBufferProperties);

	ADeviceBackend::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = deviceBackend->_device,
		._commandPool	= deviceContext->getCommandPool(),
		._graphicsQueue = deviceContext->getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _vertexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	this->getLogger().info() << "Vertex buffer created and data copied "
								"successfully. Cleaning up staging buffer...";

	vkDestroyBuffer(deviceBackend->_device, stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend->_device, stagingBufferMemory, nullptr);

	this->getLogger().info() << "Vertex buffer created successfully.";

	this->createIndexBuffer(deviceContext, indices);
}

evan::GPUMesh::~GPUMesh()
{
	this->getLogger().info()
		<< "GPUMesh destructor called. GPU resources should be cleaned up "
		   "using the destroy() method with the appropriate Vulkan device.";
}

////////////////////
// Public Methods //
////////////////////

void evan::GPUMesh::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying GPUMesh resources...";

	this->getLogger().info()
		<< "Destroying vertex buffer and freeing memory...";
	vkDestroyBuffer(device, _vertexBuffer, nullptr);

	this->getLogger().info() << "Freeing vertex buffer memory...";
	vkFreeMemory(device, _vertexBufferMemory, nullptr);

	this->getLogger().info() << "Destroying index buffer and freeing memory...";
	vkDestroyBuffer(device, _indexBuffer, nullptr);

	this->getLogger().info() << "Freeing index buffer memory...";
	vkFreeMemory(device, _indexBufferMemory, nullptr);
}

/////////////
// Getters //
/////////////

VkBuffer evan::GPUMesh::getVertexBuffer() const
{
	return _vertexBuffer;
}

VkBuffer evan::GPUMesh::getIndexBuffer() const
{
	return _indexBuffer;
}

uint32_t evan::GPUMesh::getIndexCount() const
{
	return _indexCount;
}

uint32_t evan::GPUMesh::getMaterialID() const
{
	return _materialID;
}

///////////////////////
// Protected methods //
///////////////////////

void evan::GPUMesh::createIndexBuffer(
	std::shared_ptr<DeviceContext> deviceContext, std::vector<uint32_t> indices)
{
	this->getLogger().info()
		<< "Creating index buffer with " << indices.size() << " indices...";

	auto deviceBackend = deviceContext->getDeviceBackend();

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	this->getLogger().info() << "Index buffer size: " << bufferSize;

	if (bufferSize == 0) {
		this->getLogger().warning()
			<< "Index buffer size is zero. Skipping index buffer creation.";
		return;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._size		 = bufferSize,
		._usage		 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	this->getLogger().info() << "Creating staging buffer for index data...";

	deviceBackend->createBuffer(stagingBufferProperties);

	void *data;

	this->getLogger().info()
		<< "Mapping staging buffer memory and copying index data...";
	vkMapMemory(deviceBackend->_device, stagingBufferMemory, 0, bufferSize, 0,
				&data);
	memcpy(data, indices.data(), (size_t)bufferSize);

	this->getLogger().info() << "Unmapping staging buffer memory...";
	vkUnmapMemory(deviceBackend->_device, stagingBufferMemory);

	ADeviceBackend::CreateBufferProperties indexBufferProperties = {
		._size = bufferSize,
		._usage =
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _indexBuffer,
		._bufferMemory = _indexBufferMemory
	};
	deviceBackend->createBuffer(indexBufferProperties);

	ADeviceBackend::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = deviceBackend->_device,
		._commandPool	= deviceContext->getCommandPool(),
		._graphicsQueue = deviceContext->getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _indexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	this->getLogger().info()
		<< "Destroying staging buffer and freeing memory...";
	vkDestroyBuffer(deviceBackend->_device, stagingBuffer, nullptr);
	this->getLogger().info() << "Freeing staging buffer memory...";
	vkFreeMemory(deviceBackend->_device, stagingBufferMemory, nullptr);
}
