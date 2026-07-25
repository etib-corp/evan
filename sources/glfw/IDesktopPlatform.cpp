/*
** ETIB PROJECT, 2026
** evan
** File description:
** IDesktopPlatform
*/

#include <cmath>
#include <math.h>

#include "evan/glfw/IDesktopPlatform.hpp"

evan::IDesktopPlatform::~IDesktopPlatform()
{
	this->getLogger().info()
		<< "Destroying IDesktopPlatform and releasing associated resources...";

	glfwDestroyWindow(_window);
	glfwTerminate();
}

////////////////////
// Public Methods //
////////////////////

std::shared_ptr<evan::ADeviceBackend>
	evan::IDesktopPlatform::createDeviceBackend() const
{
	this->getLogger().info()
		<< "Creating device backend for Desktop platform...";
	return std::make_shared<DesktopBackend>(*this);
}

std::shared_ptr<evan::ASwapchainContext>
	evan::IDesktopPlatform::createSwapchainContext(
		const DeviceContext &deviceContext) const
{
	this->getLogger().info()
		<< "Creating swapchain context for Desktop platform...";
	auto swapchainContext =
		std::make_shared<DesktopSwapchainContext>(deviceContext, _window);

	auto viewportSize = swapchainContext->getViewportSize();
	utility::graphic::ViewF view;
	view.setViewportSize(
		utility::math::Vector2F{viewportSize.x / 2, viewportSize.y / 2});
	this->getLogger().error()
		<< "Setting up view for Desktop platform with viewport size: "
		<< viewportSize;
	view.setClippingPlanes(1.0f, 4000.0f);
	view.setPerspective(M_PI_2, viewportSize.x / viewportSize.y);

	this->getLogger().info() << "Created view for Desktop platform: " << view;

	swapchainContext->setView(0, view);
	return swapchainContext;
}

std::vector<std::shared_ptr<utility::event::Event>>
	evan::IDesktopPlatform::pollEvents(ADeviceBackend &deviceBackend)
{
	this->getLogger().info() << "Polling events for Desktop platform...";
	glfwPollEvents();

	std::vector<std::shared_ptr<utility::event::Event>> events;

	// Poll current key states for continuous movement
	this->getLogger().info()
		<< "Polling keyboard state for continuous movement...";
	auto pressedKeys = getPressedMovementKeys();
	for (auto &keyEvent: pressedKeys) {
		events.push_back(std::move(keyEvent));
	}

	this->getLogger().info()
		<< "Processing keyboard events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_keyboardEvents.begin()),
				  std::make_move_iterator(_keyboardEvents.end()));
	// Clear keyboard events after processing them
	_keyboardEvents.clear();

	this->getLogger().info()
		<< "Processing mouse button events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_mouseButtonEvents.begin()),
				  std::make_move_iterator(_mouseButtonEvents.end()));

	// Clear mouse button events after processing them
	_mouseButtonEvents.clear();

	this->getLogger().info()
		<< "Processing mouse motion events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_mouseMotionEvents.begin()),
				  std::make_move_iterator(_mouseMotionEvents.end()));

	// Clear mouse motion events after processing them
	_mouseMotionEvents.clear();

	this->getLogger().info()
		<< "Processing mouse wheel events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_mouseWheelEvents.begin()),
				  std::make_move_iterator(_mouseWheelEvents.end()));

	// Clear mouse wheel events after processing them
	_mouseWheelEvents.clear();

	this->getLogger().info()
		<< "Processing cursor enter/leave events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_cursorEnterEvents.begin()),
				  std::make_move_iterator(_cursorEnterEvents.end()));

	// Clear cursor enter/leave events after processing them
	_cursorEnterEvents.clear();

	this->getLogger().info()
		<< "Processing file drop events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_fileDropEvents.begin()),
				  std::make_move_iterator(_fileDropEvents.end()));

	// Clear file drop events after processing them
	_fileDropEvents.clear();

	this->getLogger().info()
		<< "Processing text input events for Desktop platform...";
	events.insert(events.end(),
				  std::make_move_iterator(_textInputEvents.begin()),
				  std::make_move_iterator(_textInputEvents.end()));

	// Clear text input events after processing them
	_textInputEvents.clear();

	return events;
}

std::vector<std::string>
	evan::IDesktopPlatform::getRequiredInstanceExtensions() const
{
	this->getLogger().info()
		<< "Getting required instance extensions for Desktop platform...";

	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char *> extensions(glfwExtensions,
										 glfwExtensions + glfwExtensionCount);

	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	if (enableValidationLayers == true) {
		this->getLogger().info() << "Validation layers are enabled. Adding "
									"VK_EXT_DEBUG_UTILS_EXTENSION_NAME to "
									"required instance extensions.";
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return std::vector<std::string>(extensions.begin(), extensions.end());
}

bool evan::IDesktopPlatform::shouldClose() const
{
	this->getLogger().info() << "Checking if Desktop window should close...";
	return glfwWindowShouldClose(_window);
}

utility::event::KeyboardEvent::KeyCode
	evan::IDesktopPlatform::convertGlfwKeyToKeyCode(int glfwKey) const
{
	return static_cast<utility::event::KeyboardEvent::KeyCode>(glfwKey);
}

utility::event::MouseButtonEvent::Button
	evan::IDesktopPlatform::convertGlfwMouseButtonToButton(int glfwButton) const
{
	this->getLogger().info()
		<< "Converting GLFW mouse button to event button...";
	switch (glfwButton) {
		case GLFW_MOUSE_BUTTON_LEFT:
			return utility::event::MouseButtonEvent::Button::Left;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return utility::event::MouseButtonEvent::Button::Right;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return utility::event::MouseButtonEvent::Button::Middle;
		case GLFW_MOUSE_BUTTON_4:
			return utility::event::MouseButtonEvent::Button::X1;
		case GLFW_MOUSE_BUTTON_5:
			return utility::event::MouseButtonEvent::Button::X2;
		default:
			return utility::event::MouseButtonEvent::Button::Unknown;
	}
}

utility::event::KeyboardEvent::KeyModifiers
	evan::IDesktopPlatform::convertGlfwModsToKeyModifiers(int glfwMods) const
{
	std::uint16_t result = 0;

	if (glfwMods & GLFW_MOD_SHIFT) {
		result |= static_cast<std::uint16_t>(
			utility::event::KeyboardEvent::KeyModifiers::Shift);
	}
	if (glfwMods & GLFW_MOD_CONTROL) {
		result |= static_cast<std::uint16_t>(
			utility::event::KeyboardEvent::KeyModifiers::Ctrl);
	}
	if (glfwMods & GLFW_MOD_ALT) {
		result |= static_cast<std::uint16_t>(
			utility::event::KeyboardEvent::KeyModifiers::Alt);
	}
	if (glfwMods & GLFW_MOD_SUPER) {
		result |= static_cast<std::uint16_t>(
			utility::event::KeyboardEvent::KeyModifiers::Gui);
	}

	return static_cast<utility::event::KeyboardEvent::KeyModifiers>(result);
}

utility::event::MouseMotionEvent::MousePosition
	evan::IDesktopPlatform::getMousePosition() const
{
	double xPos = 0.0;
	double yPos = 0.0;

	glfwGetCursorPos(_window, &xPos, &yPos);
	return utility::event::MouseMotionEvent::MousePosition {
		static_cast<float>(xPos), static_cast<float>(yPos)
	};
}

std::vector<std::shared_ptr<utility::event::KeyboardEvent>>
	evan::IDesktopPlatform::getPressedMovementKeys() const
{
	std::vector<std::shared_ptr<utility::event::KeyboardEvent>> events;
	const int movementKeys[] = { GLFW_KEY_W,	GLFW_KEY_S,	   GLFW_KEY_A,
								 GLFW_KEY_D,	GLFW_KEY_Q,	   GLFW_KEY_E,
								 GLFW_KEY_UP,	GLFW_KEY_DOWN, GLFW_KEY_LEFT,
								 GLFW_KEY_RIGHT };

	for (int key: movementKeys) {
		if (glfwGetKey(_window, key) == GLFW_PRESS) {
			auto event = std::make_shared<utility::event::KeyboardEvent>();
			event->setKeycode(convertGlfwKeyToKeyCode(key));
			event->setIsDownEvent(true);
			events.push_back(std::move(event));
		}
	}

	return events;
}
