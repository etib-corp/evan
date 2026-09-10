/*
** ETIB PROJECT, 2026
** evan
** File description:
** Engine
*/

#include <typeindex>
#include <limits>
#include <map>

#include <utility/event/quit_event.hpp>
#include <utility/event/keyboard_event.hpp>
#include <utility/event/mouse_motion_event.hpp>
#include <utility/event/mouse_button_event.hpp>

#include "evan/Engine.hpp"
#include "evan/RenderObject.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

evan::Engine::Engine(
	std::shared_ptr<utility::RessourceProvider> ressourceProvider,
	std::shared_ptr<IPlatform> platform)
	: _platform(platform)
	, _ressourceProvider(ressourceProvider)
	, _lastFrameTime(std::chrono::steady_clock::now())
	, _deltaTime(0.0f)
{
	this->getLogger().info() << "Loading text shader...";

	std::string shaderPrefix = "shaders/";

#ifdef DEV
	shaderPrefix = "shaders/debug/";
	this->getLogger().info()
		<< "Debug mode enabled. Using debug shader prefix: " << shaderPrefix;
#endif

	ressourceProvider->loadShader(shaderPrefix + "text.vert.spv",
								  shaderPrefix + "text.frag.spv");

	this->getLogger().info() << "Loading default shader...";

	ressourceProvider->loadShader(shaderPrefix + "default.vert.spv",
								  shaderPrefix + "default.frag.spv");

	this->getLogger().info() << "Loading mesh shader...";

	ressourceProvider->loadShader(shaderPrefix + "mesh.vert.spv",
								  shaderPrefix + "mesh.frag.spv");

	_deviceContext	  = std::make_shared<DeviceContext>(*platform);
	_swapchainContext = platform->createSwapchainContext(*_deviceContext);

	auto deviceBackend = _deviceContext->getDeviceBackend();

	_ressourceManager =
		std::make_shared<RessourceManager>(ressourceProvider, _deviceContext);
	_renderer = std::make_shared<Renderer>(
		_deviceContext, _swapchainContext->getRenderPass(),
		_swapchainContext->getMsaaSamples(), _ressourceManager);
	_ressourceManager->init(_renderer);
}

evan::Engine::~Engine()
{
	this->getLogger().info()
		<< "Destroying engine and cleaning up resources...";

	auto deviceBackend = _deviceContext->getDeviceBackend();
	auto device		   = deviceBackend->getDevice();

	this->getLogger().info()
		<< "Waiting for device to be idle before cleanup...";
	vkDeviceWaitIdle(device);

	_swapchainContext->destroy(device);
	// Release materials, textures and shaders before the renderer destroys the
	// descriptor pool and uniform buffers they reference. The renderer owns the
	// registered render objects and destroys their GPU meshes in its destroy().
	_ressourceManager->cleanup();
	_renderer->destroy(device);
	_renderer.reset();
	_ressourceManager.reset();
	_deviceContext.reset();
	this->getLogger().info()
		<< "Engine destroyed and resources cleaned up successfully.";
}

////////////////////
// Public Methods //
////////////////////

size_t evan::Engine::addText(std::shared_ptr<utility::graphic::Text> text)
{
	if (text->getContent().empty()) {
		this->getLogger().warning()
			<< "Attempted to add text with empty content. Skipping.";
		return 0;	 // Skip adding empty text
	}

	this->getLogger().info() << "Drawing text: " << text->getContent();

	std::map<uint32_t, utility::graphic::Mesh> rawObjects;
	auto material_id = _ressourceProvider->getMaterialID(
		text->getFontFamily() + "_" + std::to_string(text->getFontSize())
		+ "_material");

	if (material_id == 0) {
		this->getLogger().warning()
			<< "Material '" << text->getFontFamily() << "_"
			<< text->getFontSize()
			<< "' not found for text object. Text will not be rendered.";
		return 0;	 // Skip rendering this text if its material is not found
	}

	this->getLogger().info()
		<< "Converting text meshes to raw objects for rendering...";
	for (const auto &mesh: text->getMeshes()) {
		this->getLogger().debug()
			<< "Processing mesh with " << mesh->getVertices().size()
			<< " vertices and " << mesh->getIndices().size() << " indices.";
		rawObjects.emplace(material_id, *mesh);
	}

	this->getLogger().info() << "Creating RenderObject for text...";
	std::shared_ptr<RenderObject> textObject =
		std::make_shared<RenderObject>(_deviceContext, rawObjects, "text");

	this->getLogger().info() << "Adding text RenderObject to engine...";
	auto objectID = _renderer->addObject(textObject);
	_ressourceManager->sync();
	return objectID;
}

size_t evan::Engine::addPrimitive(
	std::shared_ptr<utility::graphic::Primitive> primitive)
{
	if (!primitive || primitive->getMeshes().empty()) {
		this->getLogger().warning()
			<< "Attempted to add an empty primitive. Skipping.";
		return 0;
	}

	this->getLogger().info() << "Drawing primitive with "
							 << primitive->getMeshes().size() << " meshes.";

	auto material_id = _ressourceProvider->getMaterialID("mesh_material");

	std::map<uint32_t, utility::graphic::Mesh> rawObjects;
	for (const auto &mesh: primitive->getMeshes()) {
		rawObjects.emplace(material_id, *mesh);
	}

	std::shared_ptr<RenderObject> primitiveObject =
		std::make_shared<RenderObject>(_deviceContext, rawObjects, "mesh");
	auto objectID = _renderer->addObject(primitiveObject);
	_ressourceManager->sync();

	return objectID;
}

size_t evan::Engine::addModel(std::shared_ptr<utility::graphic::Model> model)
{
	auto modelTypeStr = std::string(
		model->type() == utility::graphic::Model::ModelType::OBJ ? "OBJ"
																 : "Unknown");

	this->getLogger().info() << "Drawing model of type: " << modelTypeStr;

	auto material_id = model->getMaterialID();

	std::map<uint32_t, utility::graphic::Mesh> rawObjects;
	for (const auto &mesh: model->getMeshes()) {
		rawObjects.emplace(material_id, *mesh);
	}

	std::shared_ptr<RenderObject> modelObject =
		std::make_shared<RenderObject>(_deviceContext, rawObjects, "mesh");
	auto objectID = _renderer->addObject(modelObject);
	_ressourceManager->sync();

	return objectID;
}

size_t evan::Engine::addObject(
	std::shared_ptr<utility::graphic::Renderable> object,
	const std::string &renderMethod)
{
	if (!object || object->getMeshes().empty()) {
		this->getLogger().warning()
			<< "Attempted to add an empty renderable object. Skipping.";
		return 0;
	}

	this->getLogger().info()
		<< "Drawing renderable object with " << object->getMeshes().size()
		<< " meshes using render method: " << renderMethod;

	auto material_id = _ressourceProvider->getMaterialID("mesh_material");

	std::map<uint32_t, utility::graphic::Mesh> rawObjects;
	for (const auto &mesh: object->getMeshes()) {
		rawObjects.emplace(material_id, *mesh);
	}

	const std::string pipelineLayer =
		renderMethod.empty() ? "mesh" : renderMethod;
	std::shared_ptr<RenderObject> renderObject = std::make_shared<RenderObject>(
		_deviceContext, rawObjects, pipelineLayer);
	auto objectID = _renderer->addObject(renderObject);
	_ressourceManager->sync();

	return objectID;
}

size_t evan::Engine::addMesh(const utility::graphic::Mesh &mesh,
							 const std::string &materialName,
							 const std::string &shader)
{
	std::map<uint32_t, utility::graphic::Mesh> rawObjects;
	auto material_id = _ressourceProvider->getMaterialID(materialName);
	rawObjects.emplace(material_id, mesh);
	std::shared_ptr<RenderObject> meshObject =
		std::make_shared<RenderObject>(_deviceContext, rawObjects, shader);
	auto objectID = _renderer->addObject(meshObject);
	_ressourceManager->sync();
	return objectID;
}

bool evan::Engine::removeObject(size_t objectID)
{
	this->getLogger().info() << "Removing renderable object with ID "
							 << objectID << " from engine...";
	return _renderer->removeObject(objectID);
}

utility::graphic::ViewF evan::Engine::getView(void) const
{
	const std::size_t viewCount = _swapchainContext->getViewCount();

	if (viewCount == 0) {
		throw std::runtime_error("No views available in swapchain context");
	}

	if (viewCount == 1) {
		return _swapchainContext->getView(0);
	}

	const auto leftView	 = _swapchainContext->getView(0);
	const auto rightView = _swapchainContext->getView(1);

	const auto &leftPose  = leftView.getPose();
	const auto &rightPose = rightView.getPose();

	utility::graphic::PositionF centerPosition(
		(leftPose.getPosition().getX() + rightPose.getPosition().getX()) * 0.5f,
		(leftPose.getPosition().getY() + rightPose.getPosition().getY()) * 0.5f,
		(leftPose.getPosition().getZ() + rightPose.getPosition().getZ())
			* 0.5f);

	glm::quat leftQ(leftPose.getOrientation().w, leftPose.getOrientation().x,
					leftPose.getOrientation().y, leftPose.getOrientation().z);

	glm::quat rightQ(rightPose.getOrientation().w, rightPose.getOrientation().x,
					 rightPose.getOrientation().y,
					 rightPose.getOrientation().z);

	glm::quat centerQ = glm::normalize(glm::slerp(leftQ, rightQ, 0.5f));

	utility::graphic::OrientationF centerOrientation(centerQ.x, centerQ.y,
													 centerQ.z, centerQ.w);

	utility::graphic::PoseF centerPose(centerPosition, centerOrientation);

	return utility::graphic::ViewF(
		centerPose, leftView.getFieldOfView(), leftView.getViewportSize(),
		leftView.getNearPlane(), leftView.getFarPlane());
}

evan::Error evan::Engine::update()
{
	updateDeltaTime();
	this->getLogger().info() << "Updating engine state...";

	// Handle viewport input if capture is enabled
	if (_shouldCaptureViewportInput) {
		handleViewportInput(_capturedViewportEvents);
		_capturedViewportEvents.clear();
	}

	return Error::Ok;
}

evan::Error evan::Engine::render()
{
	this->getLogger().info() << "Starting render process...";

	return _renderer->drawFrame(*_deviceContext, *_swapchainContext);
}

evan::Error evan::Engine::getLastError() const
{
	return _platform->getLastError();
}

std::vector<std::shared_ptr<utility::event::Event>> evan::Engine::pollEvents()
{
	this->getLogger().info() << "Polling events from platform...";

	utility::event::QuitEvent::Factory quitEventFactory;
	auto events = _platform->pollEvents(*_deviceContext->getDeviceBackend());

	if (_platform->shouldClose())
		events.emplace_back(quitEventFactory.create());

	if (_shouldCaptureViewportInput) {
		this->getLogger().info()
			<< "Captured " << events.size() << " events for viewport input.";
		_capturedViewportEvents.insert(_capturedViewportEvents.end(),
									   events.begin(), events.end());

	} else {
		this->getLogger().info()
			<< "Captured " << events.size()
			<< " events, but viewport input capture is disabled.";
	}

	return events;
}

void evan::Engine::handleViewportInput(
	const std::vector<std::shared_ptr<utility::event::Event>> &events)
{
	auto currentView = getView();
	auto position	 = currentView.getPose().getPosition();
	auto orientation = currentView.getPose().getOrientation();

	bool isRightMouseButtonPressed = _isRightMouseButtonPressed;
	auto lastMousePosition		   = _lastMousePosition;

	for (const auto &event: events) {
		if (auto keyboardEvent =
				std::dynamic_pointer_cast<utility::event::KeyboardEvent>(
					event)) {
			handleKeyboardMovement(keyboardEvent, orientation, position, 100.0f,
								   _deltaTime);
			continue;
		}

		if (auto mouseButtonEvent =
				std::dynamic_pointer_cast<utility::event::MouseButtonEvent>(
					event)) {
			handleMouseButtonEvent(mouseButtonEvent, isRightMouseButtonPressed,
								   lastMousePosition);
			continue;
		}

		if (auto mouseMotionEvent =
				std::dynamic_pointer_cast<utility::event::MouseMotionEvent>(
					event)) {
			handleMouseMotionEvent(mouseMotionEvent, isRightMouseButtonPressed,
								   lastMousePosition, orientation, 0.1f,
								   _deltaTime);
			continue;
		}

		if (auto handMotionEvent =
				std::dynamic_pointer_cast<utility::event::HandMotionEvent>(
					event)) {
			handleHandMotionEvent(handMotionEvent, position, orientation,
								  100.0f, 0.1f, _deltaTime);
			continue;
		}
	}

	_isRightMouseButtonPressed = isRightMouseButtonPressed;
	_lastMousePosition		   = lastMousePosition;

	utility::graphic::PoseF updatedPose(position, orientation);

	for (size_t i = 0; i < _swapchainContext->getViewCount(); ++i) {
		auto view = _swapchainContext->getView(i);
		utility::graphic::ViewF updatedView(
			updatedPose, view.getFieldOfView(), view.getViewportSize(),
			view.getNearPlane(), view.getFarPlane());
		_swapchainContext->setView(i, updatedView);
	}
}

void evan::Engine::handleKeyboardMovement(
	const std::shared_ptr<utility::event::KeyboardEvent> &keyboardEvent,
	const utility::graphic::OrientationF &orientation,
	utility::graphic::PositionF &position, float movementSpeed, float deltaTime)
{
	if (!keyboardEvent->getIsDownEvent())
		return;

	auto keycode = keyboardEvent->getKeycode();
	glm::quat q(orientation.w, orientation.x, orientation.y, orientation.z);

	glm::vec3 forward = glm::normalize(q * glm::vec3(0.0f, 0.0f, -1.0f));
	glm::vec3 right	  = glm::normalize(q * glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 up	  = glm::normalize(q * glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 movement(0.0f);

	switch (keycode) {
		case utility::event::KeyboardEvent::KeyCode::W:
		case utility::event::KeyboardEvent::KeyCode::Up:
			movement = forward;
			break;
		case utility::event::KeyboardEvent::KeyCode::S:
		case utility::event::KeyboardEvent::KeyCode::Down:
			movement = -forward;
			break;
		case utility::event::KeyboardEvent::KeyCode::A:
		case utility::event::KeyboardEvent::KeyCode::Left:
			movement = -right;
			break;
		case utility::event::KeyboardEvent::KeyCode::D:
		case utility::event::KeyboardEvent::KeyCode::Right:
			movement = right;
			break;
		case utility::event::KeyboardEvent::KeyCode::Q:
			movement = -up;
			break;
		case utility::event::KeyboardEvent::KeyCode::E:
			movement = up;
			break;
		default:
			return;
	}

	movement *= movementSpeed * deltaTime;

	position = utility::graphic::PositionF(position.getX() + movement.x,
										   position.getY() + movement.y,
										   position.getZ() + movement.z);
}

void evan::Engine::handleMouseButtonEvent(
	const std::shared_ptr<utility::event::MouseButtonEvent> &mouseButtonEvent,
	bool &isRightMouseButtonPressed, utility::math::Vector2F &lastMousePosition)
{
	if (mouseButtonEvent->getButton()
		== utility::event::MouseButtonEvent::Button::Right) {
		isRightMouseButtonPressed = mouseButtonEvent->isButtonPressed();
		if (isRightMouseButtonPressed) {
			lastMousePosition = mouseButtonEvent->getPosition();
		}
	}
}

void evan::Engine::handleMouseMotionEvent(
	const std::shared_ptr<utility::event::MouseMotionEvent> &mouseMotionEvent,
	bool isRightMouseButtonPressed, utility::math::Vector2F &lastMousePosition,
	utility::graphic::OrientationF &orientation, float rotationSpeed,
	float deltaTime)
{
	auto currentPosition = mouseMotionEvent->getPosition();
	utility::graphic::RayF ray;

	try {
		ray = getView().viewPointToRay(currentPosition);
	} catch (const std::out_of_range &exception) {
		this->getLogger().warning()
			<< "MouseButton::update: " << exception.what();
		return;
	}

	updateDebugRay(ray);

	if (!isRightMouseButtonPressed) {
		return;
	}

	auto deltaX = static_cast<float>(static_cast<int>(currentPosition.x)
									 - static_cast<int>(lastMousePosition.x));
	auto deltaY = static_cast<float>(static_cast<int>(currentPosition.y)
									 - static_cast<int>(lastMousePosition.y));

	// Create rotation from mouse delta
	glm::quat currentQuat(orientation.w, orientation.x, orientation.y,
						  orientation.z);

	// Rotate around Y axis (yaw) for horizontal movement
	glm::quat yawRotation = glm::angleAxis(deltaX * rotationSpeed * deltaTime,
										   glm::vec3(0.0f, 1.0f, 0.0f));

	// Rotate around X axis (pitch) for vertical movement
	glm::quat pitchRotation = glm::angleAxis(deltaY * rotationSpeed * deltaTime,
											 glm::vec3(1.0f, 0.0f, 0.0f));

	glm::quat newOrientation = yawRotation * pitchRotation * currentQuat;
	newOrientation			 = glm::normalize(newOrientation);

	orientation = utility::graphic::OrientationF(
		newOrientation.x, newOrientation.y, newOrientation.z, newOrientation.w);

	lastMousePosition = currentPosition;
}

void evan::Engine::handleHandMotionEvent(
	const std::shared_ptr<utility::event::HandMotionEvent> &handMotionEvent,
	utility::graphic::PositionF &position,
	utility::graphic::OrientationF &orientation, float movementSpeed,
	float rotationSpeed, float deltaTime)
{
	utility::graphic::RayF handRay;
	handRay.setOrigin(handMotionEvent->getAim().getPosition());
	handRay.setDirection(
		handMotionEvent->getAim().getOrientation().getForward());
	updateDebugRay(handRay);
}

void evan::Engine::updateDebugRay(const utility::graphic::RayF &ray)
{
	utility::graphic::Mesh rayMesh =
		ray.convertToMesh(10.0f, 0.01f, 16, { 0, 255, 0, 255 });

	if (_debugRayObjectID == std::numeric_limits<size_t>::max()) {
		_debugRayObjectID = this->addMesh(rayMesh, "mesh_material");
		return;
	}

	auto renderObject = _renderer->getObject(_debugRayObjectID);
	if (!renderObject) {
		// The debug ray object no longer exists. Recreate it.
		_debugRayObjectID = this->addMesh(rayMesh, "mesh_material");
		return;
	}

	if (renderObject->getMeshes().empty()) {
		return;
	}

	std::vector<GPUVertex> gpuVertices;
	gpuVertices.reserve(rayMesh.getVertices().size());
	for (const auto &vertex: rayMesh.getVertices()) {
		gpuVertices.push_back(GPUVertex::createFromVertex(vertex));
	}

	renderObject->getMeshes().front()->updateVertices(gpuVertices);
}

void evan::Engine::updateDeltaTime(void)
{
	static constexpr auto targetFrameTime =
		std::chrono::duration<float>(1.0f / 60.0f);

	auto currentTime = std::chrono::steady_clock::now();
	auto elapsed	 = currentTime - _lastFrameTime;

	if (elapsed < targetFrameTime) {
		std::this_thread::sleep_for(targetFrameTime - elapsed);
		currentTime = std::chrono::steady_clock::now();
		elapsed		= currentTime - _lastFrameTime;
	}

	_deltaTime	   = std::chrono::duration<float>(elapsed).count();
	_lastFrameTime = currentTime;
}

float evan::Engine::getDeltaTime(void) const
{
	return _deltaTime;
}