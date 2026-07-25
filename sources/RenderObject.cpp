/*
** ETIB PROJECT, 2026
** xider
** File description:
** RenderObject
*/

#include "evan/RenderObject.hpp"

evan::RenderObject::RenderObject(
	std::shared_ptr<DeviceContext> deviceContext,
	const std::map<uint32_t, utility::graphic::Mesh> &rawObjects,
	const std::string &pipelineLayer)
	: _pipelineLayer(pipelineLayer)
{
	this->getLogger().info()
		<< "Creating RenderObject with pipeline layer: " << pipelineLayer;

	this->getLogger().info()
		<< "Processing raw mesh data to create GPU meshes...";
	for (const auto &[id, mesh]: rawObjects) {
		this->getLogger().info()
			<< "Processing mesh with ID: " << id
			<< " and vertex count: " << mesh.getVertices().size()
			<< " and index count: " << mesh.getIndices().size();
		std::vector<GPUVertex> gpuVertices;

		this->getLogger().info()
			<< "Converting vertices to GPUVertex format...";
		for (const auto &vertex: mesh.getVertices()) {
			this->getLogger().info() << "Converting vertex with position: ("
					+ std::to_string(vertex.getPosition().getX()) + ", "
					+ std::to_string(vertex.getPosition().getY()) + ", "
					+ std::to_string(vertex.getPosition().getZ())
					+ ") and normal: (" + std::to_string(vertex.getNormal().x)
					+ ", " + std::to_string(vertex.getNormal().y) + ", "
					+ std::to_string(vertex.getNormal().z) + ") and texCoord: ("
					+ std::to_string(vertex.getTextureCoordinates().x) + ", "
					+ std::to_string(vertex.getTextureCoordinates().y) + ")"
					+ " and color: ("
					+ std::to_string(vertex.getColor().getRed()) + ", "
					+ std::to_string(vertex.getColor().getGreen()) + ", "
					+ std::to_string(vertex.getColor().getBlue()) + ", "
					+ std::to_string(vertex.getColor().getAlpha()) + ")";
			GPUVertex gpuVertex = GPUVertex::createFromVertex(vertex);
			gpuVertices.push_back(gpuVertex);
		}

		this->getLogger().info() << "Creating GPUMesh for mesh ID: " << id;

		_meshes.emplace_back(std::make_shared<GPUMesh>(
			deviceContext, gpuVertices, mesh.getIndices(), id));
	}
	this->getLogger().info()
		<< "All meshes processed and GPU meshes created successfully.";
}

evan::RenderObject::~RenderObject()
{
	this->getLogger().info()
		<< "Destroying RenderObject with pipeline layer: " << _pipelineLayer;
}

////////////////////
// Public methods //
////////////////////

void evan::RenderObject::destroy(VkDevice device)
{
	this->getLogger().info()
		<< "Destroying RenderObject resources for pipeline layer: "
		<< _pipelineLayer;
	for (const auto &mesh: _meshes) {
		mesh->destroy(device);
	}
}

/////////////
// Getters //
/////////////

const std::vector<std::shared_ptr<evan::GPUMesh>> &
	evan::RenderObject::getMeshes() const
{
	return _meshes;
}

std::string evan::RenderObject::getPipelineLayer() const
{
	return _pipelineLayer;
}
