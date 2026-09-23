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

	for (const auto &vertex : vertices) {
		_bounds.include(utility::math::Vector<float, 3>(vertex.pos));
	}

	this->getLogger().info()
		<< "GPUMesh set up with " << vertices.size() << " vertices and "
		<< indices.size() << " indices. Creating vertex buffer...";

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	this->getLogger().info() << "Vertex buffer size: " << bufferSize;
	if (bufferSize == 0) {
		this->getLogger().warning() << "Vertex buffer size is zero.";
		return;
	}

	ADeviceBackend::CreateBufferProperties vertexBufferProperties = {
		._size	= bufferSize,
		._usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
			| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _vertexBuffer,
		._bufferMemory = _vertexBufferMemory
	};
	deviceBackend->createBuffer(vertexBufferProperties);

	this->getLogger().info() << "Uploading vertex data to the GPU...";

	deviceContext->getTransferManager().uploadBuffer(
		vertices.data(), bufferSize, _vertexBuffer);

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

	VkDeviceSize bufferSize = sizeof(GPUVertex) * vertices.size();

	this->getLogger().info() << "Uploading vertex data to the GPU...";
	_deviceContext->getTransferManager().uploadBuffer(
		vertices.data(), bufferSize, _vertexBuffer);

	utility::math::AabbF bounds;
	for (const auto &vertex : vertices) {
		bounds.include(utility::math::Vector<float, 3>(vertex.pos));
	}
	_bounds = bounds;
}

///////////////////////
// Protected methods //
///////////////////////

void evan::GPUMesh::cleanup()
{
	if (!_deviceContext || !_deviceContext->getDeviceBackend()) {
		return;
	}

	// Block until pending uploads referencing these buffers have completed.
	_deviceContext->getTransferManager().waitIdle();

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

const utility::math::AabbF &evan::GPUMesh::getBounds() const
{
	return _bounds;
}

void evan::GPUMesh::setTransform(const glm::mat4 &transform)
{
	_transform = transform;
}

const glm::mat4 &evan::GPUMesh::getTransform() const
{
	return _transform;
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

	ADeviceBackend::CreateBufferProperties indexBufferProperties = {
		._size = bufferSize,
		._usage =
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		._properties   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		._buffer	   = _indexBuffer,
		._bufferMemory = _indexBufferMemory
	};
	deviceBackend->createBuffer(indexBufferProperties);

	this->getLogger().info() << "Uploading index data to the GPU...";
	deviceContext->getTransferManager().uploadBuffer(
		indices.data(), bufferSize, _indexBuffer);
}
