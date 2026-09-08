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
	: _deviceContext(deviceContext)
{
	this->getLogger().info()
		<< "Initializing GPUMesh with material ID: " << materialID << "...";

	auto deviceBackend = deviceContext->getDeviceBackend();
	_indexCount		   = indices.size();
	_vertexCount	   = vertices.size();
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
	vkMapMemory(deviceBackend->getDevice(), stagingBufferMemory, 0, bufferSize, 0,
				&data);
	memcpy(data, vertices.data(), (size_t)bufferSize);

	this->getLogger().info() << "Unmapping staging buffer memory...";

	vkUnmapMemory(deviceBackend->getDevice(), stagingBufferMemory);

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
		._logicalDevice = deviceBackend->getDevice(),
		._commandPool	= deviceContext->getCommandPool(),
		._graphicsQueue = deviceContext->getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _vertexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	this->getLogger().info() << "Vertex buffer created and data copied "
								"successfully. Cleaning up staging buffer...";

	vkDestroyBuffer(deviceBackend->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend->getDevice(), stagingBufferMemory, nullptr);

	this->getLogger().info() << "Vertex buffer created successfully.";

	this->createIndexBuffer(deviceContext, indices);
}

evan::GPUMesh::~GPUMesh()
{
	this->getLogger().info() << "Destroying GPUMesh...";
	this->cleanup();
}

////////////////////
// Public Methods //
////////////////////

void evan::GPUMesh::destroy(VkDevice device)
{
	(void)device;
	this->cleanup();
}

void evan::GPUMesh::updateVertices(const std::vector<GPUVertex> &vertices)
{
	if (!_deviceContext || !_deviceContext->getDeviceBackend()) {
		this->getLogger().warning()
			<< "Cannot update vertices: device context unavailable.";
		return;
	}

	if (_vertexBuffer == VK_NULL_HANDLE
		|| _vertexBufferMemory == VK_NULL_HANDLE) {
		this->getLogger().warning()
			<< "Cannot update vertices: vertex buffer not initialized.";
		return;
	}

	if (vertices.size() != _vertexCount) {
		this->getLogger().warning()
			<< "Cannot update vertices: expected " << _vertexCount
			<< " vertices but received " << vertices.size() << ".";
		return;
	}

	auto deviceBackend = _deviceContext->getDeviceBackend();
	VkDeviceSize bufferSize = sizeof(GPUVertex) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
	ADeviceBackend::CreateBufferProperties stagingBufferProperties = {
		._size		 = bufferSize,
		._usage		 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		._properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		._buffer	   = stagingBuffer,
		._bufferMemory = stagingBufferMemory
	};

	deviceBackend->createBuffer(stagingBufferProperties);

	void *data = nullptr;
	vkMapMemory(deviceBackend->getDevice(), stagingBufferMemory, 0, bufferSize,
				0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(deviceBackend->getDevice(), stagingBufferMemory);

	ADeviceBackend::CopyBufferProperties copyBufferProperties = {
		._logicalDevice = deviceBackend->getDevice(),
		._commandPool	= _deviceContext->getCommandPool(),
		._graphicsQueue = _deviceContext->getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _vertexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	vkDestroyBuffer(deviceBackend->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(deviceBackend->getDevice(), stagingBufferMemory, nullptr);
}

///////////////////////
// Protected methods //
///////////////////////

void evan::GPUMesh::cleanup()
{
	if (!_deviceContext || !_deviceContext->getDeviceBackend()) {
		return;
	}

	VkDevice device = _deviceContext->getDeviceBackend()->getDevice();

	this->getLogger().info()
		<< "Destroying vertex buffer and freeing memory...";
	if (_vertexBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, _vertexBuffer, nullptr);
		_vertexBuffer = VK_NULL_HANDLE;
	}
	if (_vertexBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, _vertexBufferMemory, nullptr);
		_vertexBufferMemory = VK_NULL_HANDLE;
	}

	this->getLogger().info() << "Destroying index buffer and freeing memory...";
	if (_indexBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, _indexBuffer, nullptr);
		_indexBuffer = VK_NULL_HANDLE;
	}
	if (_indexBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, _indexBufferMemory, nullptr);
		_indexBufferMemory = VK_NULL_HANDLE;
	}
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
	vkMapMemory(deviceBackend->getDevice(), stagingBufferMemory, 0, bufferSize, 0,
				&data);
	memcpy(data, indices.data(), (size_t)bufferSize);

	this->getLogger().info() << "Unmapping staging buffer memory...";
	vkUnmapMemory(deviceBackend->getDevice(), stagingBufferMemory);

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
		._logicalDevice = deviceBackend->getDevice(),
		._commandPool	= deviceContext->getCommandPool(),
		._graphicsQueue = deviceContext->getGraphicsQueue(),
		._srcBuffer		= stagingBuffer,
		._dstBuffer		= _indexBuffer,
		._size			= bufferSize
	};
	deviceBackend->copyBuffer(copyBufferProperties);

	this->getLogger().info()
		<< "Destroying staging buffer and freeing memory...";
	vkDestroyBuffer(deviceBackend->getDevice(), stagingBuffer, nullptr);
	this->getLogger().info() << "Freeing staging buffer memory...";
	vkFreeMemory(deviceBackend->getDevice(), stagingBufferMemory, nullptr);
}
