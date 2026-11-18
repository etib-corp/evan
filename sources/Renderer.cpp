/*
** ETIB PROJECT, 2026
** evan
** File description:
** Renderer
*/

#include "evan/Renderer.hpp"

#include "evan/Bounds.hpp"
#include "evan/Frustum.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace
{
	/**
	 * @brief Per-frame draw statistics, emitted once per command buffer
	 * instead of per mesh.
	 */
	struct DrawStats {
		std::size_t totalMeshes		= 0;
		std::size_t visibleMeshes	= 0;
		std::size_t culledMeshes	= 0;
		std::size_t drawCalls		= 0;
		std::size_t instancedDraws	= 0;
		std::size_t pipelineBinds	= 0;
		std::size_t descriptorBinds = 0;
		std::size_t skippedMeshes	= 0;
	};

	/**
	 * @brief Highest material id the renderer caches in a flat array.
	 *
	 * Provider material ids are small sequential counters, but material
	 * variants are allocated from RessourceManager::kFirstMaterialVariantID
	 * (0x80000000) upward. Caching by id therefore needs two tiers: a flat,
	 * id-indexed array for the dense range, and a small map for everything
	 * above it, so a variant id never makes the renderer allocate a slot per
	 * possible id.
	 */
	constexpr uint32_t kDenseMaterialIDCount = 1u << 16;

	/**
	 * @brief Whether verbose per-mesh draw logging is enabled.
	 *
	 * Controlled by the EVAN_DEBUG_DRAW_LOG environment variable. Off by
	 * default; set to any non-empty, non-"0" value to re-enable per-mesh
	 * trace output and the aggregate draw statistics.
	 */
	bool isDrawLogEnabled()
	{
		static const bool enabled = [] {
			const char *value = std::getenv("EVAN_DEBUG_DRAW_LOG");
			return value != nullptr && value[0] != '\0' && value[0] != '0';
		}();
		return enabled;
	}

	/**
	 * @brief Whether frustum/distance culling is disabled.
	 *
	 * Controlled by the EVAN_DEBUG_DISABLE_CULLING environment variable. Off
	 * by default; set to any non-empty, non-"0" value to draw every mesh
	 * regardless of its bounds.
	 */
	bool isCullingDisabledEnv()
	{
		static const bool disabled = [] {
			const char *value = std::getenv("EVAN_DEBUG_DISABLE_CULLING");
			return value != nullptr && value[0] != '\0' && value[0] != '0';
		}();
		return disabled;
	}

	/**
	 * @brief Face-culling policy requested through EVAN_CULL_MODE.
	 *
	 * Accepts `none`, `back` and `front`, so the cost of face culling can be
	 * A/B measured without a rebuild. Unset, empty or unrecognized values keep
	 * the construction-time policy.
	 *
	 * @param fallback Policy used when the environment does not select one.
	 * @return The effective face-culling policy.
	 */
	evan::CullMode cullModeFromEnv(evan::CullMode fallback)
	{
		const char *value = std::getenv("EVAN_CULL_MODE");

		if (value == nullptr || value[0] == '\0') {
			return fallback;
		}
		if (std::strcmp(value, "none") == 0) {
			return evan::CullMode::None;
		}
		if (std::strcmp(value, "back") == 0) {
			return evan::CullMode::Back;
		}
		if (std::strcmp(value, "front") == 0) {
			return evan::CullMode::Front;
		}
		return fallback;
	}

	/**
	 * @brief Human-readable name of a face-culling mode, for log records.
	 *
	 * @param mode The mode to name.
	 * @return "none", "back" or "front".
	 */
	const char *cullModeName(evan::CullMode mode) noexcept
	{
		switch (mode) {
			case evan::CullMode::None:
				return "none";
			case evan::CullMode::Front:
				return "front";
			case evan::CullMode::Back:
				break;
		}
		return "back";
	}

	/**
	 * @brief Formats a phase average as milliseconds, or "--" when empty.
	 *
	 * Building strings only happens in the report path: one aggregated log
	 * record per window, never on the per-frame hot path.
	 */
	std::string formatPhaseMs(const evan::PhaseStats &phase)
	{
		if (phase.samples == 0) {
			return "--";
		}
		std::ostringstream stream;
		stream << std::fixed << std::setprecision(2)
			   << static_cast<double>(phase.averageNs()) / 1000000.0;
		return stream.str();
	}
}	 // namespace

namespace evan
{
	/**
	 * @brief A mesh resolved for command buffer recording.
	 *
	 * Built once per command buffer, sorted so that opaque draws run first,
	 * consecutive draws share pipeline and descriptor state, and overdraw is
	 * reduced, then recorded with bind commands issued only when the bound
	 * state changes.
	 */
	struct DrawItem {
		uint32_t pipelineKey			= 0;
		uint32_t materialID				= 0;
		evan::BlendMode blendMode		= evan::BlendMode::Opaque;
		VkPipeline pipeline				= VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet	= VK_NULL_HANDLE;
		VkBuffer vertexBuffer			= VK_NULL_HANDLE;
		VkBuffer indexBuffer			= VK_NULL_HANDLE;
		uint32_t indexCount				= 0;
		///< Distance from the camera to the mesh bounds, used to order opaque
		///< draws front-to-back and alpha-blended draws back-to-front.
		float depth			= 0.0f;
		glm::mat4 transform = glm::mat4(1.0f);
		///< Whether the shader of this draw consumes the per-instance model
		///< matrix, which is what makes merging a run of draws correct.
		bool instanceInputs = false;
		///< Length of the run of consecutive sorted draws sharing this item's
		///< pipeline, material and geometry. Resolved while the sorted list is
		///< built, so the encode loop never re-compares those fields: it walks
		///< run heads and jumps by this length.
		uint32_t runLength = 1;
	};

	/**
	 * @brief World-space extents of one drawn mesh, computed once per frame.
	 *
	 * The model matrix of a mesh does not depend on the view, so the box of
	 * its local bounds, its center and its bounding radius are transformed
	 * once per object per frame and only tested per view. In stereo that
	 * removes one matrix/box transform and one square root from every
	 * per-view pass.
	 */
	struct MeshWorldBounds {
		utility::math::AabbF box {};
		glm::vec3 center { 0.0f };
		float radius = 0.0f;

		/**
		 * @brief Whether the bounds are usable for culling.
		 */
		[[nodiscard]] bool isEmpty() const noexcept
		{
			return box.isEmpty();
		}
	};

	/**
	 * @brief Draw state of one material, resolved once per material change.
	 *
	 * Provider material IDs are dense, so most of these live in a flat vector
	 * indexed by ID (variant IDs, allocated above that range, use a small
	 * map). Resolving a material means a material lookup, a shader blend-mode
	 * lookup, a pipeline lookup, a pipeline-layout lookup and a descriptor-set
	 * read: four hash/map lookups per draw before this cache. The entry is
	 * also the place where a material that cannot be drawn is recorded, so
	 * the failure is not retried for every mesh using it.
	 */
	struct MaterialDrawState {
		VkPipeline pipeline				= VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		///< Descriptor set of the material in each frame slot, so the cache
		///< survives the rotation of the per-frame slots.
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets {};
		std::size_t descriptorSetCount = 0;
		uint32_t pipelineKey		   = 0;
		evan::BlendMode blendMode	   = evan::BlendMode::Opaque;
		bool instanceInputs			   = false;
		///< Whether the material resolved into something drawable.
		bool drawable = false;
		///< Whether the entry was resolved for the current generation.
		bool resolved = false;
	};

	/**
	 * @brief Reusable per-frame scratch for the draw-list build.
	 *
	 * Owned by the Renderer. Every container keeps its capacity across
	 * frames, so after warm-up the frame build performs no heap allocations
	 * for them. Lists are kept per view slot so future parallel recording
	 * has one set of scratch per recorder.
	 */
	struct DrawListScratch {
		///< One draw list per view slot, in the order the meshes were built.
		std::array<std::vector<DrawItem>, MAX_SWAPCHAINS> drawLists;

		///< The same draws in recording order: the sort permutes the list once
		///< instead of moving the items through every comparison and swap.
		std::array<std::vector<DrawItem>, MAX_SWAPCHAINS> sortedDrawLists;

		///< Sort key of each draw of drawLists, parallel to it.
		std::array<std::vector<evan::DrawSortKey>, MAX_SWAPCHAINS> drawKeys;

		///< Draw-list indices, ordered by drawKeys.
		std::array<std::vector<uint32_t>, MAX_SWAPCHAINS> drawOrder;

		///< One frustum list per view slot.
		std::array<std::vector<Frustum>, MAX_SWAPCHAINS> frustums;

		///< Flattened mesh list of the current frame (raw pointers; the
		///< registry keeps the objects alive for the frame's duration).
		std::vector<GPUMesh *> meshes;

		///< World transform of each flattened mesh, parallel to meshes.
		///< A mesh may be shared between objects, so its transform belongs to
		///< the object that registered it, not to the mesh itself.
		std::vector<glm::mat4> transforms;

		///< World-space extents of each flattened mesh, parallel to meshes and
		///< shared by every view of the frame.
		std::vector<MeshWorldBounds> worldBounds;

		///< View list of the current multiview pass.
		std::vector<utility::graphic::ViewF> views;

		///< Material draw state for the dense material id range, indexed by
		///< material id.
		std::vector<MaterialDrawState> materials;

		///< Material draw state for ids above that range (material variants),
		///< see kDenseMaterialIDCount.
		std::unordered_map<uint32_t, MaterialDrawState> variantMaterials;

		///< Material generation both caches above were resolved for.
		uint64_t materialGeneration = 0;
	};
}	 // namespace evan

evan::Renderer::Renderer(std::shared_ptr<DeviceContext> deviceContext,
						 VkRenderPass renderPass,
						 VkSampleCountFlagBits msaaSamples,
						 std::size_t perFrameSlotCount,
						 std::shared_ptr<RessourceManager> ressourceManager,
						 CullMode cullMode)
	: _deviceContext(deviceContext)
	, _ressourceManager(ressourceManager)
	, _cullMode(cullModeFromEnv(cullMode))
{
	this->getLogger().info() << "Initializing Renderer...";

	_cullingEnabled = !isCullingDisabledEnv();

	this->getLogger().info()
		<< "Face culling: " << cullModeName(_cullMode) << " (front face: "
		<< (kFrontFace == VK_FRONT_FACE_CLOCKWISE ? "clockwise"
												  : "counter-clockwise")
		<< ").";

	_ressourceManager->sync();
	this->_currentFrameIndex = 0;

	this->getLogger().info() << "Current frame index: " << _currentFrameIndex;

	_drawScratch = std::make_unique<DrawListScratch>();

	this->createDescriptorSetLayout(
		deviceContext->getDeviceBackend()->getDevice());
	this->createGraphicsPipelines(
		deviceContext->getDeviceBackend()->getDevice(), renderPass,
		msaaSamples);
	this->createDescriptorPool(
		deviceContext->getDeviceBackend()->getDevice(),
		1000);	  // TODO: Change this with the AssetManager when it will be
				  // implemented
	this->getLogger().info() << "Renderer initialized successfully.";

	this->getLogger().info() << "Creating frames for rendering...";
	for (int frameIndex = 0; frameIndex < MAX_FRAMES_IN_FLIGHT; frameIndex++) {
		_frames.emplace_back(
			std::make_shared<Frame>(deviceContext, perFrameSlotCount));
	}
	this->getLogger().info() << "Frames created successfully.";

	// The frame collection is fixed from this point on, so cache the uniform
	// buffer handles: getUniformBuffers() hands out a reference instead of
	// rebuilding the vector per call.
	_uniformBuffers.reserve(_frames.size());
	for (const auto &frame: _frames) {
		_uniformBuffers.push_back(frame->getUniformBuffer());
	}

	_stats.enabled = statsEnabled();
	if (_stats.enabled) {
		// GPU timings need a timestamp query pool on every frame slot, i.e.
		// EVAN_STATS_GPU on and a device that supports timestamp queries.
		_stats.gpuAvailable = !_frames.empty();
		for (const auto &frame: _frames) {
			_stats.gpuAvailable =
				_stats.gpuAvailable && frame->hasTimestampQueries();
		}
		if (_stats.gpuAvailable) {
			_timestampPeriod =
				deviceContext->getDeviceBackend()->getTimestampPeriod();
			_timestampValidBits =
				deviceContext->getDeviceBackend()->getTimestampValidBits();
		}
		this->getLogger().info()
			<< "Frame statistics enabled: interval=" << statsLogInterval()
			<< " frame(s), gpu timestamps="
			<< (_stats.gpuAvailable ? "on" : "off") << ".";
	}
}

evan::Renderer::~Renderer()
{
	this->getLogger().info() << "Destroying Renderer...";
}

void evan::Renderer::destroy(VkDevice device)
{
	this->getLogger().info() << "Destroying Renderer resources...";

	this->getLogger().info() << "Destroying render objects...";
	_objects.forEach(
		[device](std::size_t, const std::shared_ptr<RenderObject> &object) {
			object->destroy(device);
		});
	_objects.clear();

	// The objects above released their meshes. Entries left in the geometry
	// cache would only be expired weak references to the destroyed device
	// allocations, so drop them explicitly.
	_deviceContext->getGeometryCache().clear();

	this->getLogger().info() << "Destroying descriptor pool...";
	vkDestroyDescriptorPool(device, _descriptorPool, nullptr);

	this->getLogger().info() << "Destroying graphics pipelines...";
	for (const auto &[variant, pipeline]: _pipelines) {
		this->getLogger().info()
			<< "Destroying pipeline for shader " << variant.shaderID
			<< " (blend mode "
			<< (variant.blendMode == BlendMode::Alpha ? "alpha" : "opaque")
			<< ", cull mode " << cullModeName(variant.cullMode) << ")";
		vkDestroyPipeline(device, pipeline, nullptr);
	}
	_pipelines.clear();

	// Pipeline layouts reference the descriptor set layout, so they have to be
	// destroyed before it.
	this->getLogger().info() << "Destroying pipeline layouts...";
	for (const auto &[shaderID, layout]: _pipelineLayouts) {
		vkDestroyPipelineLayout(device, layout, nullptr);
	}
	_pipelineLayouts.clear();

	this->getLogger().info() << "Destroying descriptor set layout...";
	vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);

	this->getLogger().info() << "Destroying frames...";
	for (const auto &frame: _frames) {
		frame->destroy(device);
	}
}

size_t evan::Renderer::addObject(std::shared_ptr<RenderObject> object)
{
	const size_t objectID = _objects.add(std::move(object));
	this->getLogger().info() << "Registering render object with ID " << objectID
							 << " in Renderer...";
	return objectID;
}

bool evan::Renderer::updateObject(std::shared_ptr<RenderObject> object,
								  size_t objectID)
{
	this->getLogger().info()
		<< "Updating render object with ID " << objectID << " in Renderer...";
	if (!_objects.update(objectID, std::move(object))) {
		this->getLogger().warning() << "Render object with ID " << objectID
									<< " not found in Renderer. Update failed.";
		return false;
	}
	return true;
}

bool evan::Renderer::removeObject(size_t objectID)
{
	this->getLogger().info()
		<< "Removing render object with ID " << objectID << " from Renderer...";
	return _objects.remove(objectID);
}

std::shared_ptr<evan::RenderObject>
	evan::Renderer::getObject(size_t objectID) const
{
	return _objects.get(objectID);
}

evan::Error evan::Renderer::drawFrame(const DeviceContext &deviceContext,
									  ASwapchainContext &swapchainContext)
{
	this->getLogger().info() << "Drawing frame...";

	auto device = deviceContext.getDeviceBackend()->getDevice();

	// Submit any pending uploads so the draw command buffers recorded below are
	// ordered after them on the graphics queue.
	{
		evan::ScopedTimer timer = _stats.timer(_stats.transferFlush);
		_deviceContext->getTransferManager().flush();
	}

	Error preprocessError;
	{
		evan::ScopedTimer timer = _stats.timer(_stats.preprocess);
		preprocessError =
			deviceContext.getDeviceBackend()->preprocessFrame(swapchainContext);
	}
	if (preprocessError != Error::Ok) {
		this->getLogger().warning()
			<< "Preprocessing frame failed. Skipping frame rendering.";
		return preprocessError;
	}

	if (swapchainContext.needsSwapchainRecreation()) {
		this->getLogger().warning()
			<< "Swapchain marked for recreation (window resized). Recreating "
			   "swapchain.";
		swapchainContext.recreateSwapchain(deviceContext,
										   swapchainContext.getRenderPass());
		return Error::SwapchainOutOfDate;
	}

	auto &frame						 = *_frames[_currentFrameIndex];
	const ViewSet &viewSet			 = swapchainContext.getViewSet();
	const std::size_t swapchainCount = swapchainContext.getSwapchainCount();

	this->getLogger().info()
		<< "Rendering " << viewSet.size() << " view(s) across "
		<< swapchainCount << " swapchain(s).";

	const bool waitOnImageAvailable =
		swapchainContext.usesImageAvailableSemaphore();

	// 1. Wait for the previous frame using this slot to finish on the GPU.
	// This guarantees the command buffer, the uniform buffer and the
	// semaphores are free to be reused for this frame. The fence is only
	// reset right before the first queue submission, so that early returns
	// (e.g. swapchain recreation) leave it signaled and the next call does
	// not deadlock.
	this->getLogger().info() << "Waiting for in-flight fence...";
	VkResult fenceResult;
	{
		evan::ScopedTimer timer = _stats.timer(_stats.inFlightWait);
		fenceResult =
			vkWaitForFences(device, 1, &frame._inFlight, VK_TRUE, UINT64_MAX);
	}
	if (fenceResult != VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to wait for in-flight fence. Aborting frame rendering.";
		return mapVkResult(fenceResult);
	}

	// The submissions that wrote this frame slot's timestamp queries have
	// completed now; fold them into the statistics without blocking.
	this->readGpuTimestamps(frame);

	// Flatten the registered meshes into the frame scratch once: the vectors
	// keep their capacity across frames and raw pointers avoid copying every
	// shared_ptr per view. The registry keeps the objects alive for the whole
	// frame. The transform is read from the object, not from the mesh: one
	// GPUMesh may be shared by several objects, each with its own transform.
	// The world-space bounds are transformed here, once per object, because
	// they are the same for every view: the per-view passes only test them.
	std::vector<GPUMesh *> &meshes			  = _drawScratch->meshes;
	std::vector<glm::mat4> &transforms		  = _drawScratch->transforms;
	std::vector<MeshWorldBounds> &worldBounds = _drawScratch->worldBounds;
	meshes.clear();
	transforms.clear();
	worldBounds.clear();
	_objects.forEach(
		[&meshes, &transforms, &worldBounds](
			std::size_t, const std::shared_ptr<RenderObject> &object) {
			const glm::mat4 &transform = object->getTransform();

			for (const auto &mesh: object->getMeshes()) {
				meshes.push_back(mesh.get());
				transforms.push_back(transform);

				MeshWorldBounds extent;
				extent.box = transformedBounds(mesh->getBounds(), transform);
				if (!extent.isEmpty()) {
					const auto center = extent.box.center();
					extent.center	  = glm::vec3(center.x, center.y, center.z);
					extent.radius	  = extent.box.radius();
				}
				worldBounds.push_back(extent);
			}
		});

	// Every drawn mesh consumes one transform slot per view. Grow the instance
	// buffer before recording any view: reallocating it while a recorded
	// command buffer of this frame still referenced the old handle would be
	// invalid, and the wait above guarantees the previous frame using this slot
	// has finished with the current buffer.
	frame.ensureInstanceCapacity(meshes.size());

	// 2. Acquire exactly one image per swapchain image set.
	if (swapchainCount > static_cast<std::size_t>(MAX_SWAPCHAINS)) {
		this->getLogger().error()
			<< "Frame has " << swapchainCount << " swapchains, but only "
			<< MAX_SWAPCHAINS << " per-frame slots are available. Aborting "
			<< "frame rendering.";
		return Error::RuntimeError;
	}
	std::array<uint32_t, MAX_SWAPCHAINS> acquiredImage {};
	{
		evan::ScopedTimer acquireTimer = _stats.timer(_stats.acquire);
		for (std::size_t s = 0; s < swapchainCount; ++s) {
			this->getLogger().info()
				<< "Acquiring swapchain image for swapchain " << s << "...";
			VkResult result = swapchainContext.aquireImage(
				static_cast<uint32_t>(s), device, frame._imageAvailable[s],
				VK_NULL_HANDLE, acquiredImage[s]);

			swapchainContext.waitForImage(static_cast<uint32_t>(s));

			Error acquireError = mapVkResult(result);
			if (acquireError == Error::SwapchainOutOfDate
				|| acquireError == Error::Suboptimal) {
				this->getLogger().warning()
					<< "Swapchain " << s
					<< " is out of date. Recreating swapchain.";
				swapchainContext.recreateSwapchain(
					deviceContext, swapchainContext.getRenderPass());
				return acquireError;
			} else if (acquireError != Error::Ok) {
				this->getLogger().error()
					<< "Failed to acquire swapchain image for swapchain " << s
					<< ". Aborting frame rendering.";
				return acquireError;
			}
		}
	}

	// 3. Render the frame. In multiview mode every view renders from the
	// single layered swapchain in one pass with one submission. Otherwise each
	// view owns a command buffer and a uniform buffer slot, so all views can
	// be recorded and submitted back-to-back without waiting for the GPU
	// between them. The in-flight fence is signaled only by the last
	// submission (the only one in multiview mode): the graphics queue executes
	// submissions in order, so its completion implies every earlier view has
	// completed as well. viewOwnsInFlightFence() below encodes that decision
	// and is covered by a unit test; do not attach the fence to every view,
	// that reintroduces the P0-03 inter-eye stall.
	std::array<std::size_t, MAX_SWAPCHAINS> renderableViewData {};
	std::size_t renderableViewCount = 0;
	bool tooManyRenderableViews		= false;
	for (std::size_t v = 0; v < viewSet.size(); ++v) {
		const std::size_t s = viewSet[v].swapchainIndex;
		if (s >= swapchainCount) {
			this->getLogger().error()
				<< "View " << v << " references invalid swapchain index " << s
				<< ". Skipping view.";
			continue;
		}
		if (renderableViewCount == static_cast<std::size_t>(MAX_SWAPCHAINS)) {
			tooManyRenderableViews = true;
			break;
		}
		renderableViewData[renderableViewCount++] = v;
	}

	if (tooManyRenderableViews) {
		this->getLogger().error()
			<< "Frame has more than " << MAX_SWAPCHAINS
			<< " renderable views, but only " << MAX_SWAPCHAINS
			<< " per-view resources are available. Aborting frame rendering.";
		return Error::RuntimeError;
	}

	const std::span<const std::size_t> renderableViews(
		renderableViewData.data(), renderableViewCount);

	if (!renderableViews.empty()) {
		vkResetFences(device, 1, &frame._inFlight);
	}

	if (!renderableViews.empty() && swapchainContext.usesMultiview()) {
		// Multiview: every view renders from the single layered swapchain in
		// one pass. Record one command buffer and submit it once; that single
		// submission owns the in-flight fence, so the P0-03 end-of-frame
		// contract (one GPU wait before the images are released) is kept.
		if (renderableViews.size()
			> static_cast<std::size_t>(MAX_MULTIVIEW_VIEWS)) {
			this->getLogger().error()
				<< "Multiview frame has " << renderableViews.size()
				<< " views, but only " << MAX_MULTIVIEW_VIEWS
				<< " per-view uniform slots are available. Aborting frame "
				   "rendering.";
			return Error::RuntimeError;
		}

		const std::size_t s = viewSet[renderableViews.front()].swapchainIndex;
		auto &imageSet		= *swapchainContext._swapchainImages[s];

		std::vector<utility::graphic::ViewF> &views = _drawScratch->views;
		views.clear();
		views.reserve(renderableViews.size());
		for (const std::size_t v: renderableViews) {
			views.push_back(viewSet[v].view);
		}

		{
			evan::ScopedTimer timer = _stats.timer(_stats.updateUniform);
			evan::ScopedTimer slotTimer =
				_stats.timer(_stats.updateUniformView[0]);
			this->updateMultiviewUniformBuffer(views, 0);
		}
		{
			evan::ScopedTimer timer		= _stats.timer(_stats.record);
			evan::ScopedTimer slotTimer = _stats.timer(_stats.recordView[0]);
			frame.resetCommandBuffer(0);
			this->recordCommandBuffer(swapchainContext.getRenderPass(),
									  imageSet.getFramebuffer(acquiredImage[s]),
									  imageSet.getExtent(), 0, views, meshes,
									  transforms);
		}

		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		VkSubmitInfo submitInfo {};
		submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
		submitInfo.pWaitSemaphores =
			waitOnImageAvailable ? &frame._imageAvailable[s] : nullptr;
		submitInfo.pWaitDstStageMask	= waitStages;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &frame._commandBuffers[0];
		submitInfo.signalSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
		submitInfo.pSignalSemaphores =
			waitOnImageAvailable ? &frame._renderFinished[s] : nullptr;

		VkResult submitResult;
		{
			evan::ScopedTimer timer		= _stats.timer(_stats.submit);
			evan::ScopedTimer slotTimer = _stats.timer(_stats.submitView[0]);
			submitResult = vkQueueSubmit(deviceContext.getGraphicsQueue(), 1,
										 &submitInfo, frame._inFlight);
		}
		if (submitResult != VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to submit the multiview draw command buffer. "
				   "Aborting frame rendering.";
			return mapVkResult(submitResult);
		}
	} else {
		for (std::size_t i = 0; i < renderableViews.size(); ++i) {
			const std::size_t v		  = renderableViews[i];
			const ViewSet::View &view = viewSet[v];
			const std::size_t s		  = view.swapchainIndex;

			auto &imageSet = *swapchainContext._swapchainImages[s];

			{
				evan::ScopedTimer timer = _stats.timer(_stats.updateUniform);
				evan::ScopedTimer slotTimer =
					_stats.timer(_stats.updateUniformView[i]);
				this->updateUniformBuffer(view.view, i);
			}
			{
				evan::ScopedTimer timer = _stats.timer(_stats.record);
				evan::ScopedTimer slotTimer =
					_stats.timer(_stats.recordView[i]);
				frame.resetCommandBuffer(i);
				this->recordCommandBuffer(
					swapchainContext.getRenderPass(),
					imageSet.getFramebuffer(acquiredImage[s]),
					imageSet.getExtent(), i,
					std::span<const utility::graphic::ViewF>(&view.view, 1),
					meshes, transforms);
			}

			VkPipelineStageFlags waitStages[] = {
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			};
			VkSubmitInfo submitInfo {};
			submitInfo.sType			  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
			submitInfo.pWaitSemaphores =
				waitOnImageAvailable ? &frame._imageAvailable[s] : nullptr;
			submitInfo.pWaitDstStageMask	= waitStages;
			submitInfo.commandBufferCount	= 1;
			submitInfo.pCommandBuffers		= &frame._commandBuffers[i];
			submitInfo.signalSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
			submitInfo.pSignalSemaphores =
				waitOnImageAvailable ? &frame._renderFinished[s] : nullptr;

			// Only the final view carries the in-flight fence (see the step 3
			// comment above and viewOwnsInFlightFence()).
			VkFence submitFence =
				viewOwnsInFlightFence(i, renderableViews.size())
				? frame._inFlight
				: VK_NULL_HANDLE;

			VkResult submitResult;
			{
				evan::ScopedTimer timer = _stats.timer(_stats.submit);
				evan::ScopedTimer slotTimer =
					_stats.timer(_stats.submitView[i]);
				submitResult = vkQueueSubmit(deviceContext.getGraphicsQueue(),
											 1, &submitInfo, submitFence);
			}
			if (submitResult != VK_SUCCESS) {
				this->getLogger().error()
					<< "Failed to submit draw command buffer for view " << v
					<< ". Aborting frame rendering.";
				return mapVkResult(submitResult);
			}
		}
	}

	// OpenXR does not hand a Vulkan semaphore to the runtime, so the host must
	// guarantee the render completed before the swapchain images are released
	// and xrEndFrame hands them to the compositor. This is now a single
	// end-of-frame synchronization point covering all views, instead of one
	// wait between the eyes plus another before release.
	if (!waitOnImageAvailable && !renderableViews.empty()) {
		VkResult renderWaitResult;
		{
			evan::ScopedTimer timer = _stats.timer(_stats.renderWait);
			renderWaitResult = vkWaitForFences(device, 1, &frame._inFlight,
											   VK_TRUE, UINT64_MAX);
		}
		if (renderWaitResult != VK_SUCCESS) {
			this->getLogger().error()
				<< "Failed to wait for render completion. Aborting frame "
				   "rendering.";
			return mapVkResult(renderWaitResult);
		}
	}

	// 4. Present each swapchain once, with the image acquired for it.
	{
		evan::ScopedTimer presentTimer = _stats.timer(_stats.present);
		for (std::size_t s = 0; s < swapchainCount; ++s) {
			VkPresentInfoKHR presentInfo {};
			presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = waitOnImageAvailable ? 1u : 0u;
			presentInfo.pWaitSemaphores =
				waitOnImageAvailable ? &frame._renderFinished[s] : nullptr;

			swapchainContext._swapchainImages[s]->fillPresentInfo(presentInfo);
			presentInfo.pImageIndices = &acquiredImage[s];

			Error presentError = deviceContext.getDeviceBackend()->processFrame(
				presentInfo, *swapchainContext._swapchainImages[s]);
			if (presentError == Error::SwapchainOutOfDate
				|| presentError == Error::Suboptimal) {
				this->getLogger().warning()
					<< "Swapchain " << s
					<< " is out of date. Recreating swapchain.";
				swapchainContext.recreateSwapchain(
					deviceContext, swapchainContext.getRenderPass());
				return presentError;
			} else if (presentError != Error::Ok) {
				this->getLogger().error()
					<< "Failed to present swapchain " << s << ".";
				return presentError;
			}
		}
	}

	this->getLogger().info() << "Frame drawn successfully.";

	_currentFrameIndex = (_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	this->getLogger().info() << "Next frame index: " << _currentFrameIndex;

	this->getLogger().info() << "Post-processing frame...";
	Error postprocessError;
	{
		evan::ScopedTimer timer = _stats.timer(_stats.postprocess);
		postprocessError = deviceContext.getDeviceBackend()->postprocessFrame(
			swapchainContext);
	}
	if (postprocessError != Error::Ok) {
		return postprocessError;
	}

	if (_stats.enabled) {
		++_stats.frames;
		evan::stats::takeGlobalDeltas(_stats);
		this->reportStatsIfDue();
	}

	return Error::Ok;
}

bool evan::Renderer::viewOwnsInFlightFence(std::size_t viewIndex,
										   std::size_t viewCount) noexcept
{
	return viewCount > 0 && viewIndex + 1 == viewCount;
}

void evan::Renderer::setShaderBlendMode(uint32_t shaderID, BlendMode blendMode)
{
	this->getLogger().info()
		<< "Forcing blend mode "
		<< (blendMode == BlendMode::Alpha ? "alpha" : "opaque")
		<< " for shader " << shaderID;
	_shaderBlendModes[shaderID] = blendMode;
}

void evan::Renderer::setMaterialCullMode(uint32_t materialID, CullMode cullMode)
{
	this->getLogger().info() << "Setting face culling for material "
							 << materialID << " to " << cullModeName(cullMode);
	_materialCullModes[materialID] = cullMode;
	this->invalidateMaterialCache();
}

void evan::Renderer::clearMaterialCullMode(uint32_t materialID)
{
	if (_materialCullModes.erase(materialID) == 0) {
		return;
	}
	this->getLogger().info() << "Restoring the renderer-wide face culling for "
							 << "material " << materialID;
	this->invalidateMaterialCache();
}

void evan::Renderer::invalidateMaterialCache()
{
	if (!_drawScratch) {
		return;
	}
	_drawScratch->materials.clear();
	_drawScratch->variantMaterials.clear();
	// Any generation but the next one: the next frame has to re-resolve the
	// materials whose pipeline selection changed.
	_drawScratch->materialGeneration = 0;
}

/////////////
// Getters //
/////////////

VkDescriptorPool evan::Renderer::getDescriptorPool() const
{
	return _descriptorPool;
}

const std::vector<VkBuffer> &evan::Renderer::getUniformBuffers() const
{
	return _uniformBuffers;
}

VkDescriptorSetLayout evan::Renderer::getDescriptorSetLayout() const
{
	return _descriptorSetLayout;
}

evan::BlendMode evan::Renderer::getShaderBlendMode(uint32_t shaderID) const
{
	const auto it = _shaderBlendModes.find(shaderID);
	return it != _shaderBlendModes.end() ? it->second : BlendMode::Opaque;
}

std::optional<evan::CullMode>
	evan::Renderer::getMaterialCullMode(uint32_t materialID) const
{
	const auto it = _materialCullModes.find(materialID);
	if (it == _materialCullModes.end()) {
		return std::nullopt;
	}
	return it->second;
}

VkPipeline evan::Renderer::pipelineFor(uint32_t shaderID, BlendMode blendMode,
									   CullMode cullMode) const
{
	const auto it =
		_pipelines.find(PipelineVariant { shaderID, blendMode, cullMode });
	return it != _pipelines.end() ? it->second : VK_NULL_HANDLE;
}

/////////////////////
// Private methods //
/////////////////////

void evan::Renderer::createDescriptorSetLayout(VkDevice device)
{
	this->getLogger().info() << "Creating descriptor set layout...";

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// UBO — dynamic so one buffer with per-view slots can be bound with a
	// per-view dynamic offset.
	bindings.push_back(
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
		  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr });
	this->getLogger().info()
		<< "Added uniform buffer binding to descriptor set layout.";

	// Textures -> TODO: Adapt this according to all the textures that will be
	// supported by the engine (e.g. normal map, metallic roughness map, etc.)
	bindings.push_back({ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
						 VK_SHADER_STAGE_FRAGMENT_BIT });	 // Albedo
	bindings.push_back({ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
						 VK_SHADER_STAGE_FRAGMENT_BIT });	 // Normal
	bindings.push_back(
		{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
		  VK_SHADER_STAGE_FRAGMENT_BIT });	  // MetallicRoughness

	VkDescriptorSetLayoutBinding fontAtlasBinding {};
	fontAtlasBinding.binding		 = 4;
	fontAtlasBinding.descriptorCount = 1;
	fontAtlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	fontAtlasBinding.stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.push_back(fontAtlasBinding);

	this->getLogger().info()
		<< "Added texture bindings to descriptor set layout.";

	VkDescriptorSetLayoutCreateInfo layoutInfo {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings	= bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
									&_descriptorSetLayout)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create descriptor set layout!";
		return;
	}
	this->getLogger().info() << "Descriptor set layout created successfully.";
}

void evan::Renderer::createGraphicsPipelines(VkDevice device,
											 VkRenderPass renderPass,
											 VkSampleCountFlagBits msaaSamples)
{
	this->getLogger().info() << "Creating graphics pipelines...";

	const auto &shaders = _ressourceManager->getShaders();

	this->getLogger().info() << "Iterating over shaders to create pipelines...";
	for (const auto &[id, shader]: shaders) {
		this->getLogger().info() << "Creating pipeline for shader: " << id;

		VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
		vertShaderStageInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shader->getVertexShaderModule();
		vertShaderStageInfo.pName  = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
		fragShaderStageInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shader->getFragmentShaderModule();
		fragShaderStageInfo.pName  = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo, fragShaderStageInfo
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		vertexInputInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		this->getLogger().info()
			<< "Getting vertex input descriptions for shader: " << id;
		auto bindingDescription	   = GPUVertex::getBindingDescription();
		auto instanceBinding	   = GPUVertex::getInstanceBindingDescription();
		auto attributeDescriptions = GPUVertex::getAttributeDescriptions();
		auto instanceAttributes = GPUVertex::getInstanceAttributeDescriptions();

		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			bindingDescription, instanceBinding
		};
		std::vector<VkVertexInputAttributeDescription> allAttributes;
		allAttributes.reserve(attributeDescriptions.size()
							  + instanceAttributes.size());
		allAttributes.insert(allAttributes.end(), attributeDescriptions.begin(),
							 attributeDescriptions.end());
		allAttributes.insert(allAttributes.end(), instanceAttributes.begin(),
							 instanceAttributes.end());

		vertexInputInfo.vertexBindingDescriptionCount =
			static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(allAttributes.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.pVertexAttributeDescriptions = allAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
		inputAssembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState {};
		viewportState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount	= 1;

		VkPipelineRasterizationStateCreateInfo rasterizer {};
		rasterizer.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable		   = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode			   = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth			   = 1.0f;
		// Culling and winding are static state. The front face is settled once
		// for the Y-flipping projection (see kFrontFace); the cull mode is
		// selected per variant below, so a material can opt out of culling
		// without a new dynamic state. VK_DYNAMIC_STATE_CULL_MODE is
		// deliberately absent: it belongs to VK_EXT_extended_dynamic_state
		// (Vulkan 1.3), which the device feature chain does not enable.
		rasterizer.cullMode		   = toVkCullMode(_cullMode);
		rasterizer.frontFace	   = kFrontFace;
		rasterizer.depthBiasEnable = VK_FALSE;

#ifdef DEV
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
#endif

		VkPipelineMultisampleStateCreateInfo multisampling {};
		multisampling.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable  = VK_FALSE;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading	   = .2f;

		// VK_DYNAMIC_STATE_CULL_MODE is deliberately absent: it belongs to
		// VK_EXT_extended_dynamic_state (Vulkan 1.3), the device feature chain
		// does not enable that feature, and the value was never set per frame,
		// so the state was undefined at draw time.
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState {};
		dynamicState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount =
			static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPushConstantRange pushConstantRange {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset	 = 0;
		pushConstantRange.size		 = sizeof(glm::vec4);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts	  = &_descriptorSetLayout;

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges	  = &pushConstantRange;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
								   &_pipelineLayouts[id])
			!= VK_SUCCESS) {
			this->getLogger().error() << "Failed to create pipeline layout !";
			return;
		}

		// One variant per blend mode: opaque geometry must not pay for the
		// blender, and blended geometry must not write depth. Each blend mode
		// is then paired with every static cull mode below.
		for (const BlendMode blendMode:
			 { BlendMode::Opaque, BlendMode::Alpha }) {
			const bool isAlpha = blendMode == BlendMode::Alpha;

			VkPipelineColorBlendAttachmentState colorBlendAttachment {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
				| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
				| VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = isAlpha ? VK_TRUE : VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor =
				VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor =
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp		 = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor =
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo colorBlending {};
			colorBlending.sType =
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable		= VK_FALSE;
			colorBlending.logicOp			= VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount	= 1;
			colorBlending.pAttachments		= &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			VkPipelineDepthStencilStateCreateInfo depthStencil {};
			depthStencil.sType =
				VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			// Blended draws are ordered back-to-front by the draw list, so
			// they must not occlude each other through the depth buffer.
			depthStencil.depthWriteEnable	   = isAlpha ? VK_FALSE : VK_TRUE;
			depthStencil.depthCompareOp		   = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds		   = 0.0f;
			depthStencil.maxDepthBounds		   = 1.0f;
			depthStencil.stencilTestEnable	   = VK_FALSE;
			depthStencil.front				   = {};
			depthStencil.back				   = {};

			VkGraphicsPipelineCreateInfo pipelineInfo {};
			pipelineInfo.sType =
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount			 = 2;
			pipelineInfo.pStages			 = shaderStages;
			pipelineInfo.pVertexInputState	 = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState		 = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState	 = &multisampling;
			pipelineInfo.pColorBlendState	 = &colorBlending;
			pipelineInfo.pDynamicState		 = &dynamicState;
			pipelineInfo.layout				 = _pipelineLayouts[id];
			pipelineInfo.renderPass			 = renderPass;
			pipelineInfo.subpass			 = 0;
			pipelineInfo.basePipelineHandle	 = VK_NULL_HANDLE;
			pipelineInfo.pDepthStencilState	 = &depthStencil;

			// One pipeline per cull mode: the mode is static rasterization
			// state, and having all three variants ready is what lets a
			// material opt out of culling without creating a pipeline at
			// draw time.
			for (const CullMode faceCull:
				 { CullMode::None, CullMode::Back, CullMode::Front }) {
				rasterizer.cullMode = toVkCullMode(faceCull);

				const PipelineVariant variant { id, blendMode, faceCull };
				if (vkCreateGraphicsPipelines(
						device, _deviceContext->getPipelineCache().getHandle(),
						1, &pipelineInfo, nullptr, &_pipelines[variant])
					!= VK_SUCCESS) {
					this->getLogger().error()
						<< "Failed to create the "
						<< (isAlpha ? "alpha" : "opaque")
						<< " graphics pipeline for shader " << id
						<< " with cull mode " << cullModeName(faceCull) << "!";
					return;
				}
			}
		}
		// The GPUShader modules are owned by the RessourceManager. They are
		// released when the manager replaces or drops the wrapper, so do not
		// destroy them here or RessourceManager would double-free them.
	}
	this->getLogger().info() << "Graphics pipelines created successfully.";
}

void evan::Renderer::createDescriptorPool(VkDevice device,
										  uint32_t materialCount)
{
	this->getLogger().info() << "Creating descriptor pool...";

	uint32_t descriptorCount = materialCount * MAX_FRAMES_IN_FLIGHT;

	this->getLogger().info()
		<< "Descriptor count calculated: " << descriptorCount;

	std::array<VkDescriptorPoolSize, 2> poolSizes {};
	poolSizes[0].type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = descriptorCount;
	poolSizes[1].type			 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = descriptorCount;

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	// GPUMaterial releases its descriptor sets individually, which the spec
	// only allows for a pool created with this flag.
	poolInfo.flags		   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes	   = poolSizes.data();
	poolInfo.maxSets	   = descriptorCount;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool)
		!= VK_SUCCESS) {
		this->getLogger().error() << "Failed to create descriptor pool!";
		return;
	}
	this->getLogger().info() << "Descriptor pool created successfully.";
}

void evan::Renderer::updateUniformBuffer(const utility::graphic::ViewF &view,
										 std::size_t viewSlot)
{
	this->getLogger().info()
		<< "Updating uniform buffer for current frame index: "
		<< _currentFrameIndex << ", view slot: " << viewSlot;

	// Two-pass fallback: each eye owns its slot and the vertex shader reads
	// entry 0 of the bound slot (ViewIndex is zero outside multiview passes).
	Frame::UniformBufferObject ubo {};
	ubo.model		  = glm::mat4(1.0f);
	ubo.views[0].view = view.toViewMatrix();
	ubo.views[0].proj = view.getProjectionMatrix();

	memcpy(_frames[_currentFrameIndex]->getUniformBufferMapped(viewSlot), &ubo,
		   sizeof(ubo));
	this->getLogger().info() << "Uniform buffer updated successfully.";
}

void evan::Renderer::updateMultiviewUniformBuffer(
	const std::vector<utility::graphic::ViewF> &views, std::size_t viewSlot)
{
	this->getLogger().info()
		<< "Updating multiview uniform buffer for current frame index: "
		<< _currentFrameIndex << ", view slot: " << viewSlot
		<< ", views: " << views.size();

	// Multiview: one slot holds every eye and the vertex shader selects the
	// entry through gl_ViewIndex.
	Frame::UniformBufferObject ubo {};
	ubo.model = glm::mat4(1.0f);

	const std::size_t count = views.size() < MAX_MULTIVIEW_VIEWS
		? views.size()
		: static_cast<std::size_t>(MAX_MULTIVIEW_VIEWS);
	for (std::size_t i = 0; i < count; ++i) {
		ubo.views[i].view = views[i].toViewMatrix();
		ubo.views[i].proj = views[i].getProjectionMatrix();
	}

	memcpy(_frames[_currentFrameIndex]->getUniformBufferMapped(viewSlot), &ubo,
		   sizeof(ubo));
	this->getLogger().info()
		<< "Multiview uniform buffer updated successfully.";
}

void evan::Renderer::setScissor(const utility::graphic::ScissorRect &rect)
{
	_scissor = rect;
}

void evan::Renderer::clearScissor(void)
{
	_scissor.reset();
}

VkRect2D evan::Renderer::clampScissor(const utility::graphic::ScissorRect &rect,
									  VkExtent2D extent)
{
	const float maxWidth  = static_cast<float>(extent.width);
	const float maxHeight = static_cast<float>(extent.height);

	const float x0 = std::clamp(rect.x, 0.0f, maxWidth);
	const float y0 = std::clamp(rect.y, 0.0f, maxHeight);
	const float x1 =
		std::clamp(rect.x + std::max(0.0f, rect.width), 0.0f, maxWidth);
	const float y1 =
		std::clamp(rect.y + std::max(0.0f, rect.height), 0.0f, maxHeight);

	VkRect2D result {};
	result.offset = { static_cast<int32_t>(x0), static_cast<int32_t>(y0) };
	result.extent = { static_cast<uint32_t>(std::max(0.0f, x1 - x0)),
					  static_cast<uint32_t>(std::max(0.0f, y1 - y0)) };
	return result;
}

void evan::Renderer::recordCommandBuffer(
	VkRenderPass renderPass, VkFramebuffer swapChainFramebuffer,
	VkExtent2D swapChainExtent, std::size_t viewSlot,
	std::span<const utility::graphic::ViewF> views,
	std::span<GPUMesh *const> meshes, std::span<const glm::mat4> transforms)
{
	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Recording command buffer for current frame index: "
			<< _currentFrameIndex << ", view slot: " << viewSlot;
	}

	_ressourceManager->sync();

	auto &frame		   = *_frames[_currentFrameIndex];
	auto commandBuffer = frame.getCommandBuffer(viewSlot);

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		this->getLogger().error()
			<< "Failed to begin recording command buffer!";
		return;
	}

	VkRenderPassBeginInfo renderPassInfo {};
	renderPassInfo.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass		 = renderPass;
	renderPassInfo.framebuffer		 = swapChainFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues {};
	clearValues[0].color		= { { 0.184313729f, 0.309803933f, 0.309803933f,
									  1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues	   = clearValues.data();

	// GPU timings: reset this view slot's query pair and write the begin
	// timestamp just before the render pass starts executing. The reset is
	// recorded, so it is queue-ordered after the previous frame that used
	// this slot (whose completion the in-flight fence already guarantees).
	if (frame.hasTimestampQueries()) {
		const uint32_t beginQuery = static_cast<uint32_t>(2 * viewSlot);
		vkCmdResetQueryPool(commandBuffer, frame.getTimestampQueryPool(),
							beginQuery, 2);
		vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							frame.getTimestampQueryPool(), beginQuery);
		// From now on the slot holds a reset query pair: the results may be
		// read once the in-flight fence of a later use of this slot signals.
		frame.markTimestampQueriesRecorded(viewSlot);
	}

	if (isDrawLogEnabled()) {
		this->getLogger().debug() << "Beginning render pass...";
	}
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
						 VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport {};
	viewport.x		  = 0.0f;
	viewport.y		  = 0.0f;
	viewport.width	  = (float)swapChainExtent.width;
	viewport.height	  = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Viewport set to cover entire swapchain extent: "
			<< swapChainExtent.width << "x" << swapChainExtent.height;
	}

	VkRect2D scissor {};
	if (_scissor.has_value()) {
		scissor = clampScissor(_scissor.value(), swapChainExtent);
	} else {
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
	}
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Scissor set to cover entire swapchain extent: "
			<< swapChainExtent.width << "x" << swapChainExtent.height;
	}

	DrawStats stats {};
	stats.totalMeshes = meshes.size();

	std::vector<DrawItem> &drawList = _drawScratch->drawLists[viewSlot];
	// Recording order: filled by the permutation below, so the sort never
	// moves the items themselves (each carries its model matrix).
	std::vector<DrawItem> &recordList = _drawScratch->sortedDrawLists[viewSlot];
	// Number of leading draws of recordList that are opaque; resolved by the
	// build phase so the encode phase only walks two contiguous ranges.
	std::size_t opaqueCount = 0;

	{
		evan::ScopedTimer buildTimer = _stats.timer(_stats.recordBuild);

		// A pass covers one view in the fallback path and every eye in
		// multiview mode. Culling must keep a mesh visible for any covered
		// view, and the depth order is measured from the eye midpoint (a
		// heuristic; the depth buckets make the exact reference point
		// non-critical).
		std::vector<Frustum> &frustums = _drawScratch->frustums[viewSlot];
		frustums.clear();
		frustums.reserve(views.size());
		glm::vec3 cameraPos(0.0f);
		for (const auto &view: views) {
			frustums.push_back(Frustum::fromViewProjection(
				view.getProjectionMatrix() * view.toViewMatrix()));

			const auto cameraPosition = view.getPose().getPosition();
			cameraPos +=
				glm::vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z);
		}
		if (!views.empty()) {
			cameraPos /= static_cast<float>(views.size());
		}

		// Build the draw list: cull, resolve each mesh's pipeline, material
		// and descriptor state through the material cache, then sort so
		// consecutive draws share GPU state. The list is reused across frames
		// (per view slot) with its capacity kept.
		drawList.clear();
		drawList.reserve(meshes.size());

		std::vector<MaterialDrawState> &materials = _drawScratch->materials;
		std::unordered_map<uint32_t, MaterialDrawState> &variantMaterials =
			_drawScratch->variantMaterials;
		const uint64_t materialGeneration =
			_ressourceManager->getMaterialGeneration();
		if (materialGeneration != _drawScratch->materialGeneration) {
			materials.clear();
			variantMaterials.clear();
			_drawScratch->materialGeneration = materialGeneration;
		}

		// Resolves what recording one material needs: its pipeline, pipeline
		// layout, blend mode, descriptor sets and whether its shader consumes
		// the instance matrix. Material IDs are dense, so the result is kept
		// per ID and reused by every mesh of the material, in every view,
		// until the material state changes. A material that cannot be drawn
		// is cached as such, instead of repeating the failed lookups per mesh.
		const auto resolveMaterial = [this](uint32_t materialID,
											MaterialDrawState &state) {
			state.resolved = true;
			state.drawable = false;

			const auto material = _ressourceManager->getMaterial(materialID);
			if (!material) {
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "Material with ID " << materialID
						<< " not found! Skipping its meshes.";
				}
				return;
			}

			state.pipelineKey = material->getShaderID();
			// Opaque and blended draws of the same shader use different
			// pipelines, so the variant has to be resolved before recording.
			// An explicit per-shader override wins over the material alpha
			// mode.
			const bool alpha = material->getBlendMode() == BlendMode::Alpha
				|| this->getShaderBlendMode(state.pipelineKey)
					== BlendMode::Alpha;
			state.blendMode = alpha ? BlendMode::Alpha : BlendMode::Opaque;
			// The face-culling policy is per material, defaulting to the
			// renderer-wide mode; the pipeline triple is static, so the
			// variant is picked here rather than at draw time.
			const auto cullOverride = _materialCullModes.find(materialID);
			const CullMode cullMode = cullOverride != _materialCullModes.end()
				? cullOverride->second
				: _cullMode;
			state.pipeline =
				this->pipelineFor(state.pipelineKey, state.blendMode, cullMode);
			if (state.pipeline == VK_NULL_HANDLE) {
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "No pipeline found for shader ID: "
						<< state.pipelineKey << " (blend mode "
						<< (alpha ? "alpha" : "opaque") << ", cull mode "
						<< cullModeName(cullMode) << "). Skipping its meshes.";
				}
				return;
			}

			const auto layoutIt = _pipelineLayouts.find(state.pipelineKey);
			if (layoutIt == _pipelineLayouts.end()) {
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "No pipeline layout found for shader ID: "
						<< state.pipelineKey << ". Skipping its meshes.";
				}
				return;
			}
			state.pipelineLayout = layoutIt->second;

			// Only a shader that declares the per-instance matrix can be
			// merged into an instanced draw: a shader that ignores it would
			// place every instance of the run at the same place.
			const auto shader = _ressourceManager->getShader(state.pipelineKey);
			state.instanceInputs = shader && shader->declaresInstanceInputs();

			const auto &descriptorSets = material->getDescriptorSets();
			state.descriptorSetCount   = descriptorSets.size();
			for (std::size_t slot = 0; slot < state.descriptorSets.size();
				 ++slot) {
				state.descriptorSets[slot] = slot < descriptorSets.size()
					? descriptorSets[slot]
					: VK_NULL_HANDLE;
			}
			state.drawable = true;
		};

		// Depth range of the visible draws, accumulated while the list is
		// built instead of in an extra pass over it.
		float nearestDepth	= kUnknownDepth;
		float farthestDepth = 0.0f;

		for (std::size_t meshIndex = 0; meshIndex < meshes.size();
			 ++meshIndex) {
			const GPUMesh *mesh		   = meshes[meshIndex];
			const glm::mat4 &transform = transforms[meshIndex];

			// Geometry is local-space and the model matrix carries the pose,
			// so culling and depth sorting must use the transformed bounds:
			// the local box of every mesh of a kind sits at the same place.
			// The world-space box is computed once per object per frame
			// (drawFrame), because it does not depend on the view.
			const MeshWorldBounds &extent =
				_drawScratch->worldBounds[meshIndex];

			if (_cullingEnabled) {
				if (!extent.isEmpty()) {
					if (_maxDrawDistance > 0.0f
						&& glm::distance(cameraPos, extent.center)
								- extent.radius
							> _maxDrawDistance) {
						++stats.culledMeshes;
						continue;
					}
					// Conservative across the covered views: keeping a mesh
					// that only one eye sees is correct, dropping it would
					// not be.
					bool intersectsAnyView = frustums.empty();
					for (const auto &frustum: frustums) {
						if (frustum.intersects(extent.box)) {
							intersectsAnyView = true;
							break;
						}
					}
					if (!intersectsAnyView) {
						++stats.culledMeshes;
						continue;
					}
				}
			}
			++stats.visibleMeshes;

			const uint32_t materialID = mesh->getMaterialID();
			if (isDrawLogEnabled()) {
				this->getLogger().debug()
					<< "Processing mesh with material ID: " << materialID;
			}

			MaterialDrawState *materialEntry = nullptr;
			if (materialID < kDenseMaterialIDCount) {
				if (static_cast<std::size_t>(materialID) >= materials.size()) {
					materials.resize(static_cast<std::size_t>(materialID) + 1);
				}
				materialEntry = &materials[materialID];
			} else {
				materialEntry = &variantMaterials[materialID];
			}
			MaterialDrawState &materialState = *materialEntry;
			if (!materialState.resolved) {
				resolveMaterial(materialID, materialState);
			}
			if (!materialState.drawable) {
				++stats.skippedMeshes;
				continue;
			}

			const auto vertexBuffer = mesh->getVertexBuffer();
			if (vertexBuffer == VK_NULL_HANDLE) {
				++stats.skippedMeshes;
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "Vertex buffer is null for mesh with material ID: "
						<< materialID << ". Skipping mesh.";
				}
				continue;
			}

			const auto indexBuffer = mesh->getIndexBuffer();
			if (indexBuffer == VK_NULL_HANDLE) {
				++stats.skippedMeshes;
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "Index buffer is null for mesh with material ID: "
						<< materialID << ". Skipping mesh.";
				}
				continue;
			}

			if (_currentFrameIndex >= materialState.descriptorSetCount) {
				++stats.skippedMeshes;
				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "No descriptor set for material ID " << materialID
						<< " at frame index " << _currentFrameIndex
						<< ". Skipping mesh.";
				}
				continue;
			}

			// Distance from the camera, used to order draws: front-to-back
			// for opaque geometry so early depth testing rejects occluded
			// fragments, back-to-front for blended geometry so compositing is
			// correct. Meshes without usable bounds are treated as infinitely
			// far away.
			float depth = kUnknownDepth;
			if (!extent.isEmpty()) {
				depth = glm::distance(cameraPos, extent.center);
			}
			if (depth < nearestDepth) {
				nearestDepth = depth;
			}
			if (depth != kUnknownDepth && depth > farthestDepth) {
				farthestDepth = depth;
			}

			drawList.push_back(
				{ materialState.pipelineKey, materialID,
				  materialState.blendMode, materialState.pipeline,
				  materialState.pipelineLayout,
				  materialState.descriptorSets[_currentFrameIndex],
				  vertexBuffer, indexBuffer, mesh->getIndexCount(), depth,
				  transform, materialState.instanceInputs });
		}

		// Depth buckets are normalized against the visible depth range so that
		// the bucket count stays meaningful whatever the scene scale is. Draws
		// landing in the same bucket keep their state ordering, which
		// preserves the run merging used by instanced and indirect drawing.
		const float depthSpan =
			farthestDepth > nearestDepth ? farthestDepth - nearestDepth : 0.0f;
		const bool frontToBack =
			_opaqueSortMode == OpaqueSortMode::FrontToBack && depthSpan > 0.0f;

		// Finalize the sort key once per draw. The depth bucket depends on the
		// whole list (it is normalized against the visible range), so it is
		// resolved here rather than while the list is built; the sort then
		// compares keys and never recomputes anything.
		std::vector<DrawSortKey> &keys = _drawScratch->drawKeys[viewSlot];
		keys.resize(drawList.size());
		for (std::size_t i = 0; i < drawList.size(); ++i) {
			const DrawItem &item = drawList[i];
			DrawSortKey &key	 = keys[i];

			key.blendMode	 = item.blendMode;
			key.depth		 = item.depth;
			key.pipelineKey	 = item.pipelineKey;
			key.materialID	 = item.materialID;
			key.vertexBuffer = item.vertexBuffer;
			key.indexBuffer	 = item.indexBuffer;
			// A disabled front-to-back ordering collapses every opaque draw
			// into the same bucket, which leaves the state ordering in charge.
			key.depthBucket = frontToBack
				? opaqueDepthBucket(item.depth, nearestDepth, depthSpan,
									kOpaqueDepthBucketCount)
				: 0u;
			// Unique draw-list position: the last key field, so the order is
			// total and reproducible instead of depending on std::sort's
			// tie-breaking.
			key.sequence = static_cast<uint32_t>(i);
		}

		// Opaque draws come first, so the recording pass can walk two
		// contiguous ranges. Inside a group, consecutive draws sharing
		// pipeline, material and geometry stay adjacent for batching.
		//
		// The order is sorted through an index array: comparing and swapping
		// 4-byte indices costs a fraction of moving the items, each of which
		// carries its model matrix.
		std::vector<uint32_t> &order = _drawScratch->drawOrder[viewSlot];
		order.resize(drawList.size());
		for (std::size_t i = 0; i < order.size(); ++i) {
			order[i] = static_cast<uint32_t>(i);
		}
		std::sort(order.begin(), order.end(),
				  [&keys](uint32_t lhs, uint32_t rhs) {
					  return drawSortKeyLess(keys[lhs], keys[rhs]);
				  });

		// Permute the list once into recording order and resolve the runs
		// while doing it: the run length is stored on every item of the run,
		// so the encode loop never compares pipeline, material and geometry
		// again.
		recordList.clear();
		recordList.reserve(drawList.size());
		std::size_t runStart = 0;
		for (const uint32_t index: order) {
			DrawItem item  = drawList[index];
			item.runLength = 1;
			if (!recordList.empty()) {
				const DrawItem &previous = recordList.back();
				if (previous.pipelineKey == item.pipelineKey
					&& previous.materialID == item.materialID
					&& previous.vertexBuffer == item.vertexBuffer
					&& previous.indexBuffer == item.indexBuffer) {
					++recordList[runStart].runLength;
					item.runLength = recordList[runStart].runLength;
				} else {
					runStart = recordList.size();
				}
			}
			recordList.push_back(item);
		}

		while (opaqueCount < recordList.size()
			   && recordList[opaqueCount].blendMode == BlendMode::Opaque) {
			++opaqueCount;
		}
	}

	{
		evan::ScopedTimer encodeTimer = _stats.timer(_stats.recordEncode);

		// Record the sorted draw list, re-binding state only when it changes.
		VkPipeline boundPipeline		   = VK_NULL_HANDLE;
		VkDescriptorSet boundDescriptorSet = VK_NULL_HANDLE;
		VkBuffer boundVertexBuffer		   = VK_NULL_HANDLE;
		VkBuffer boundIndexBuffer		   = VK_NULL_HANDLE;
		bool hasBoundState				   = false;

		const VkDeviceSize instanceBufferOffset =
			viewSlot * frame.getInstanceBufferAlignedSize();
		// The per-view uniform slot is a dynamic UBO offset: constant for
		// every draw of this view.
		const uint32_t dynamicOffset = static_cast<uint32_t>(
			viewSlot * frame.getUniformBufferAlignedSize());
		// Every mesh owns one transform slot, so the mapped pointer is always
		// valid: the model matrix of a mesh is uploaded at its own slot and
		// selected at draw time through firstInstance. The matrices are
		// written in one contiguous pass over the recording order instead of
		// inside each run, which keeps the writes sequential and out of the
		// command-recording loop.
		GPUInstance *instanceData =
			static_cast<GPUInstance *>(frame.getInstanceBufferMapped(viewSlot));

		// The frame sized the buffer for every registered mesh before
		// recording, so this guard only catches a caller that grew the scene
		// behind the renderer's back.
		std::size_t drawableCount		   = recordList.size();
		const std::size_t instanceCapacity = frame.getInstanceCapacity();
		if (drawableCount > instanceCapacity) {
			const std::size_t dropped = drawableCount - instanceCapacity;
			this->getLogger().error()
				<< "Instance buffer capacity exceeded (" << instanceCapacity
				<< " per view). Skipping " << dropped << " meshes.";
			stats.skippedMeshes += dropped;
			drawableCount = instanceCapacity;
		}
		for (std::size_t i = 0; i < drawableCount; ++i) {
			instanceData[i].model = recordList[i].transform;
		}

		const VkDeviceSize indirectBufferOffset =
			viewSlot * frame.getIndirectBufferAlignedSize();
		VkDrawIndexedIndirectCommand *indirectData = _indirectDrawingEnabled
			? static_cast<VkDrawIndexedIndirectCommand *>(
				  frame.getIndirectBufferMapped(viewSlot))
			: nullptr;
		uint32_t indirectCursor					   = 0;
		uint32_t indirectPendingBase			   = 0;

		const auto flushIndirect = [&]() {
			if (!_indirectDrawingEnabled
				|| indirectCursor == indirectPendingBase) {
				return;
			}
			vkCmdDrawIndexedIndirect(
				commandBuffer, frame.getIndirectBuffer(),
				indirectBufferOffset
					+ indirectPendingBase
						* sizeof(VkDrawIndexedIndirectCommand),
				indirectCursor - indirectPendingBase,
				static_cast<uint32_t>(sizeof(VkDrawIndexedIndirectCommand)));
			indirectPendingBase = indirectCursor;
		};

		const auto emitDraw = [&](uint32_t indexCount, uint32_t instanceCount,
								  uint32_t firstInstance) {
			if (_indirectDrawingEnabled
				&& indirectCursor
					< static_cast<uint32_t>(MAX_INDIRECT_COMMANDS_PER_VIEW)) {
				indirectData[indirectCursor++] = { indexCount, instanceCount, 0,
												   0, firstInstance };
			} else {
				vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0,
								 firstInstance);
			}
			++stats.drawCalls;
		};

		const auto recordRange = [&](std::size_t begin, std::size_t end) {
			if (begin == end) {
				return;
			}

			// Each pass starts from unbound state so its first draw rebinds
			// the pipeline, the buffers and the descriptor set.
			boundPipeline	   = VK_NULL_HANDLE;
			boundDescriptorSet = VK_NULL_HANDLE;
			boundVertexBuffer  = VK_NULL_HANDLE;
			boundIndexBuffer   = VK_NULL_HANDLE;
			hasBoundState	   = false;

			// The instance buffer and its offset are the same for every draw
			// of the view, so they are bound once per pass instead of being
			// re-checked and re-bound from inside the item loop.
			{
				VkBuffer instanceBuffer		   = frame.getInstanceBuffer();
				VkDeviceSize instanceOffsets[] = { instanceBufferOffset };
				vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer,
									   instanceOffsets);
			}

			for (std::size_t i = begin; i < end;) {
				const DrawItem &item = recordList[i];

				// Runs were resolved while the sorted list was built, so the
				// loop jumps from run head to run head instead of comparing
				// pipeline, material and geometry for every item. Each mesh
				// keeps its own transform slot either way, which is why the
				// first instance of a run is the run's position in the list.
				std::size_t runSize =
					(_mergeIdenticalRuns && item.instanceInputs)
					? static_cast<std::size_t>(item.runLength)
					: 1u;
				if (i + runSize > end) {
					runSize = end - i;
				}

				// The dynamic offset is constant for the whole view and the
				// pipeline layout follows the pipeline, so neither takes part
				// in the per-item state comparison.
				const bool stateChanged = !hasBoundState
					|| boundPipeline != item.pipeline
					|| boundVertexBuffer != item.vertexBuffer
					|| boundIndexBuffer != item.indexBuffer
					|| boundDescriptorSet != item.descriptorSet;

				if (stateChanged) {
					flushIndirect();
				}

				if (!hasBoundState || boundPipeline != item.pipeline) {
					if (isDrawLogEnabled()) {
						this->getLogger().debug()
							<< "Binding pipeline for shader ID: "
							<< item.pipelineKey;
					}
					vkCmdBindPipeline(commandBuffer,
									  VK_PIPELINE_BIND_POINT_GRAPHICS,
									  item.pipeline);
					boundPipeline = item.pipeline;
					++stats.pipelineBinds;

					glm::vec4 color { 1.f, 1.f, 1.f, 1.f };
					vkCmdPushConstants(commandBuffer, item.pipelineLayout,
									   VK_SHADER_STAGE_FRAGMENT_BIT, 0,
									   sizeof(glm::vec4), &color);
				}

				if (!hasBoundState || boundVertexBuffer != item.vertexBuffer) {
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(commandBuffer, 0, 1,
										   &item.vertexBuffer, offsets);
					boundVertexBuffer = item.vertexBuffer;
					if (isDrawLogEnabled()) {
						this->getLogger().debug()
							<< "Binding vertex buffer for ->..";
					}
				}

				if (!hasBoundState || boundIndexBuffer != item.indexBuffer) {
					vkCmdBindIndexBuffer(commandBuffer, item.indexBuffer, 0,
										 VK_INDEX_TYPE_UINT32);
					boundIndexBuffer = item.indexBuffer;
				}

				if (!hasBoundState
					|| boundDescriptorSet != item.descriptorSet) {
					if (isDrawLogEnabled()) {
						this->getLogger().debug()
							<< "Binding descriptor set for material ID: "
							<< item.materialID;
					}
					vkCmdBindDescriptorSets(
						commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						item.pipelineLayout, 0, 1, &item.descriptorSet, 1,
						&dynamicOffset);
					boundDescriptorSet = item.descriptorSet;
					++stats.descriptorBinds;
				}

				if (isDrawLogEnabled()) {
					this->getLogger().debug()
						<< "Drawing indexed mesh with index count: "
						<< item.indexCount
						<< " and instance count: " << runSize;
				}
				emitDraw(item.indexCount, static_cast<uint32_t>(runSize),
						 static_cast<uint32_t>(i));
				if (runSize > 1) {
					++stats.instancedDraws;
				}

				i += runSize;
				hasBoundState = true;
			}
		};

		// Every opaque draw precedes every blended one: the split was resolved
		// by the build phase, so both ranges are recorded in order without
		// scanning the list again. The blended range shares the depth buffer
		// with the opaque one but does not write to it.
		const std::size_t opaqueEnd = std::min(opaqueCount, drawableCount);
		recordRange(0, opaqueEnd);
		recordRange(opaqueEnd, drawableCount);

		flushIndirect();
	}

	// Fold the per-view draw counters into the statistics window; the
	// EVAN_DEBUG_DRAW_LOG block below keeps reading the local values.
	if (_stats.enabled) {
		_stats.totalMeshes += stats.totalMeshes;
		_stats.visibleMeshes += stats.visibleMeshes;
		_stats.culledMeshes += stats.culledMeshes;
		_stats.skippedMeshes += stats.skippedMeshes;
		_stats.drawCalls += stats.drawCalls;
		_stats.instancedDraws += stats.instancedDraws;
		_stats.pipelineBinds += stats.pipelineBinds;
		_stats.descriptorBinds += stats.descriptorBinds;
	}

	if (isDrawLogEnabled()) {
		this->getLogger().debug()
			<< "Draw stats: totalMeshes=" << stats.totalMeshes
			<< " visibleMeshes=" << stats.visibleMeshes
			<< " culledMeshes=" << stats.culledMeshes
			<< " drawCalls=" << stats.drawCalls
			<< " instancedDraws=" << stats.instancedDraws
			<< " pipelineBinds=" << stats.pipelineBinds
			<< " descriptorBinds=" << stats.descriptorBinds
			<< " skippedMeshes=" << stats.skippedMeshes;
		this->getLogger().debug() << "All meshes processed. Ending render "
									 "pass...";
	}

	vkCmdEndRenderPass(commandBuffer);

	if (frame.hasTimestampQueries()) {
		vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
							frame.getTimestampQueryPool(),
							static_cast<uint32_t>(2 * viewSlot + 1));
	}

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		this->getLogger().error() << "Failed to record command buffer!";
		return;
	}
}

void evan::Renderer::reportStatsIfDue()
{
	if (!_stats.enabled || _stats.frames < statsLogInterval()) {
		return;
	}

	auto log = this->getLogger().info();

	log << "evan stats: frames=" << _stats.frames << " | cpu_ms(avg):"
		<< " flush=" << formatPhaseMs(_stats.transferFlush)
		<< " pre=" << formatPhaseMs(_stats.preprocess)
		<< " acq=" << formatPhaseMs(_stats.acquire)
		<< " wait=" << formatPhaseMs(_stats.inFlightWait)
		<< " ubo=" << formatPhaseMs(_stats.updateUniform)
		<< " rec=" << formatPhaseMs(_stats.record)
		<< " recbuild=" << formatPhaseMs(_stats.recordBuild)
		<< " recenc=" << formatPhaseMs(_stats.recordEncode)
		<< " submit=" << formatPhaseMs(_stats.submit)
		<< " renderwait=" << formatPhaseMs(_stats.renderWait)
		<< " pres=" << formatPhaseMs(_stats.present)
		<< " post=" << formatPhaseMs(_stats.postprocess) << " | gpu_ms(avg):";

	bool anyGpuSample = false;
	for (std::size_t slot = 0; slot < static_cast<std::size_t>(MAX_SWAPCHAINS);
		 ++slot) {
		if (_stats.gpuPass[slot].samples == 0) {
			continue;
		}
		anyGpuSample = true;
		log << " pass" << slot << "=" << formatPhaseMs(_stats.gpuPass[slot]);
	}
	if (!anyGpuSample) {
		log << " --";
	}

	log << " | meshes: total=" << _stats.totalMeshes
		<< " vis=" << _stats.visibleMeshes << " culled=" << _stats.culledMeshes
		<< " skip=" << _stats.skippedMeshes
		<< " | draws: calls=" << _stats.drawCalls
		<< " inst=" << _stats.instancedDraws << " pso=" << _stats.pipelineBinds
		<< " ds=" << _stats.descriptorBinds
		<< " | sync: calls=" << _stats.syncCalls << " skip=" << _stats.syncSkips
		<< " | mesh_churn: +" << _stats.meshCreations << "/-"
		<< _stats.meshDestructions << " | uploadwaits=" << _stats.uploadWaits;

	_stats.resetWindow();
}

void evan::Renderer::readGpuTimestamps(const Frame &frame)
{
	if (!_stats.gpuAvailable || !frame.hasTimestampQueries()) {
		return;
	}

	auto device = _deviceContext->getDeviceBackend()->getDevice();

	// The per-frame slot count is clamped to [1, MAX_SWAPCHAINS], so the
	// gpuPass indices below are always in range.
	for (std::size_t slot = 0; slot < frame.getPerFrameSlotCount(); ++slot) {
		if (!frame.hasRecordedTimestampQueries(slot)) {
			// The slot recorded nothing yet (first use of the frame slot, or
			// a view count that never touched it). Reading an unreset query
			// is a validation error; skip it.
			continue;
		}

		// One result plus its availability flag per query, 64-bit each.
		uint64_t values[4]	  = { 0, 0, 0, 0 };
		const VkResult result = vkGetQueryPoolResults(
			device, frame.getTimestampQueryPool(),
			static_cast<uint32_t>(2 * slot), 2, sizeof(values), values,
			static_cast<VkDeviceSize>(2 * sizeof(uint64_t)),
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);

		if (result != VK_SUCCESS || values[1] == 0 || values[3] == 0) {
			// VK_NOT_READY: the queries of this slot were not written yet
			// (first use of the frame slot, skipped frame). Never block on a
			// query result.
			continue;
		}

		const uint64_t begin =
			evan::stats::maskTimestamp(values[0], _timestampValidBits);
		const uint64_t end =
			evan::stats::maskTimestamp(values[2], _timestampValidBits);

		uint64_t deltaNs = 0;
		if (!evan::stats::timestampDeltaNs(begin, end, _timestampPeriod,
										   deltaNs)) {
			continue;
		}
		_stats.gpuPass[slot].add(deltaNs);
	}
}