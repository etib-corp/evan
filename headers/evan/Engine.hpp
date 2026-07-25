/*
** ETIB PROJECT, 2026
** evan
** File description:
** Engine
*/

#pragma once

#include <chrono>
#include <utility/graphic/material.hpp>

#include "evan/Version.hpp"

#include "evan/DeviceContext.hpp"
#include "evan/Renderer.hpp"
#include "evan/Scene.hpp"
#include "evan/ASwapchainContext.hpp"
#include "evan/IPlatform.hpp"

#include <utility/graphic/model.hpp>
#include <utility/graphic/primitive.hpp>
#include <utility/graphic/view.hpp>
#include <utility/graphic/text/text.hpp>

#include <utility/ressource_provider.hpp>

#include <utility/event/event.hpp>
#include <utility/event/keyboard_event.hpp>
#include <utility/event/mouse_motion_event.hpp>
#include <utility/event/mouse_button_event.hpp>
#include <utility/event/hand_motion_event.hpp>

#include <utility/logging/loggable.hpp>
#include <utility/logging/default_logger.hpp>

#include "Scene.hpp"
#include "RenderObject.hpp"
#include "RessourceManager.hpp"

#include <string>
#include <memory>

namespace evan
{
	/**
	 * @brief The Engine class is the main entry point for the Evan Engine. It
	 * manages the core components of the engine, including the device context,
	 * renderer, scenes, swapchain context, and platform abstraction. The
	 * Engine class is responsible for initializing these components, running
	 * the main loop of the engine, and providing methods for updating and
	 * rendering scenes. It serves as the central hub for managing the engine's
	 * functionality and orchestrating the various subsystems to create a
	 * cohesive experience.
	 *
	 * The Engine class provides a high-level interface for interacting with
	 * the engine, allowing users to add scenes, handle updates, and manage
	 * rendering without needing to directly interact with the lower-level
	 * components. It abstracts away the complexities of Vulkan and
	 * platform-specific details, providing a more user-friendly API for
	 * developers using the Evan Engine.
	 *
	 * The Engine class is designed to be extensible and modular, allowing for
	 * future enhancements and additions to the engine's capabilities. It can be
	 * extended to support additional features such as physics, audio, or
	 * networking, while still maintaining a clear separation of concerns
	 * between different subsystems of the engine.
	 *
	 * Overall, the Engine class serves as the backbone of the Evan Engine,
	 * providing a structured and organized way to manage the various components
	 * and functionalities of the engine while offering a user-friendly
	 * interface for developers to create their applications.
	 *
	 * @note The Engine class is currently in its early stages of development,
	 * and additional features and improvements are expected to be added in
	 * future iterations of the engine. The current implementation focuses on
	 * establishing the core structure and functionality of the engine, with
	 * plans for further enhancements and optimizations in the future.
	 */
	class Engine:
		protected utility::logging::Loggable<Engine,
											 utility::logging::DefaultLogger>
	{
		public:
		/**
		 * @brief Constructs a new Engine object with default settings. This
		 * constructor initializes the engine with default parameters, such as a
		 * default window name, width, and height. It sets up the necessary
		 * components of the engine, including the device context, renderer,
		 * swapchain context, and platform abstraction, using default
		 * configurations. This constructor is useful for quickly getting
		 * started with the engine without needing to specify custom parameters.
		 *
		 * @param ressourceProvider A shared pointer to a RessourceProvider
		 * object, which is responsible for managing the loading and
		 * synchronization of GPU resources such as materials and textures. The
		 * RessourceProvider interacts with the utility::RessourceProvider to
		 * load resources from disk or other sources, and creates corresponding
		 * GPU resources using the DeviceContext. It provides methods for
		 * synchronizing resources, retrieving specific materials or textures by
		 * ID, and managing the lifecycle of GPU resources to ensure efficient
		 * memory usage and performance in the rendering process.
		 * @param platform A shared pointer to an IPlatform object, which
		 * provides an abstraction layer for platform-specific operations. The
		 * IPlatform interface defines methods for window management, input
		 * handling, and other platform-dependent functionality, allowing the
		 * engine to be portable across different operating systems and
		 * platforms.
		 *
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		Engine(std::shared_ptr<utility::RessourceProvider> ressourceProvider,
			   std::shared_ptr<IPlatform> platform);

		~Engine();

		/**
		 * @brief Adds a text object to the scene. This method takes a shared
		 * pointer to a Text object, which contains the necessary information
		 * for rendering the text, such as the string content, font, size, and
		 * color.
		 *
		 * The addText method is responsible for setting up the appropriate
		 * graphics pipeline, binding the necessary resources, and issuing the
		 * draw calls to render the text on the screen. It interacts with the
		 * Renderer and DeviceContext to ensure that the text is rendered
		 * correctly, taking into account factors such as the current scene,
		 * camera position, and any transformations applied to the text.
		 *
		 * @param text A shared pointer to the Text object to be added to the
		 * scene.
		 *
		 * @return The unique identifier (size_t) of the added text object
		 * within the scene, which can be used for future reference or
		 * manipulation of the text object in the scene.
		 */
		size_t addText(std::shared_ptr<utility::graphic::Text> text);

		/**
		 * @brief Removes a previously added render object from the current
		 * scene.
		 * @param objectID The identifier returned by addText or addMesh.
		 * @return True when the object was removed from the scene.
		 */
		bool removeObject(size_t objectID);

		/**
		 * @brief Get the mirrored view state.
		 * @return The current view.
		 */
		utility::graphic::ViewF getView(void) const;

		/**
		 * @brief Adds a primitive object to the scene. This method takes a
		 * shared pointer to a Primitive object, which contains the necessary
		 * information for rendering the primitive, such as the mesh data,
		 * material properties, and any transformations applied to the
		 * primitive.
		 *
		 * The addPrimitive method is responsible for setting up the appropriate
		 * graphics pipeline, binding the necessary resources, and issuing the
		 * draw calls to render the primitive on the screen. It interacts with
		 * the Renderer and DeviceContext to ensure that the primitive is
		 * rendered correctly, taking into account factors such as the current
		 * scene, camera position, and any transformations applied to the
		 * primitive.
		 *
		 * @param primitive A shared pointer to the Primitive object to be drawn
		 * on the screen.
		 * @return The unique identifier (size_t) of the added primitive object
		 * within the scene, which can be used for future reference or
		 * manipulation of the primitive object in the scene.
		 */
		size_t addPrimitive(
			std::shared_ptr<utility::graphic::Primitive> primitive);

		/**
		 * @brief Adds a model object to the scene. This method takes a shared
		 * pointer to a Model object, which contains the necessary information
		 * for rendering the model, such as the mesh data, material properties,
		 * and any transformations applied to the model.
		 *
		 * The addModel method is responsible for setting up the appropriate
		 * graphics pipeline, binding the necessary resources, and issuing the
		 * draw calls to render the model on the screen. It interacts with the
		 * Renderer and DeviceContext to ensure that the model is rendered
		 * correctly, taking into account factors such as the current scene,
		 * camera position, and any transformations applied to the model.
		 *
		 * @param model A shared pointer to the Model object to be added to the
		 * scene.
		 * @return The unique identifier (size_t) of the added model object
		 * within the scene, which can be used for future reference or
		 * manipulation of the model object in the scene.
		 */
		size_t addModel(std::shared_ptr<utility::graphic::Model> model);

		/**
		 * @brief Adds a generic renderable object to the scene. This method
		 * takes a shared pointer to a Renderable object, which is a base class
		 * for various types of render objects, such as Text, Primitive, and
		 * Model. The method also takes a string parameter representing the
		 * render method to be used for drawing the object, allowing for
		 * flexibility in how the object is rendered.
		 *
		 * The addObject method is responsible for determining the appropriate
		 * graphics pipeline and rendering approach based on the type of the
		 * Renderable object and the specified render method. It interacts with
		 * the Renderer and DeviceContext to ensure that the object is rendered
		 * correctly, taking into account factors such as the current scene,
		 * camera position, and any transformations applied to the object.
		 *
		 * @param object A shared pointer to the Renderable object to be added
		 * to the scene.
		 * @param renderMethod A string representing the render method to be
		 * used for drawing the object, allowing for flexibility in how the
		 * object is rendered.
		 * @return The unique identifier (size_t) of the added renderable object
		 * within the scene, which can be used for future reference or
		 * manipulation of the renderable object in the scene.
		 */
		size_t addObject(std::shared_ptr<utility::graphic::Renderable> object,
						 const std::string &renderMethod);

		/**
		 * @brief Adds a mesh to the renderer. This method takes a Mesh object,
		 * which contains the necessary information for rendering the mesh, such
		 * as the vertex data, index data, and material properties.
		 *
		 * The addMesh method is responsible for setting up the appropriate
		 * graphics pipeline, binding the necessary resources, and issuing the
		 * draw calls to render the mesh on the screen. It interacts with the
		 * Renderer and DeviceContext to ensure that the mesh is rendered
		 * correctly, taking into account factors such as the current scene,
		 * camera position, and any transformations applied to the mesh.
		 *
		 * @param mesh The Mesh object to be added to the renderer.
		 * @param materialName The name of the material to be used for rendering
		 * the mesh. This parameter allows for flexibility in specifying the
		 * material properties to be applied to the mesh during rendering.
		 * @param shader The name of the shader to be used for rendering the
		 * mesh. This parameter allows for flexibility in specifying the shader
		 * program to be used for rendering the mesh, enabling different visual
		 * effects and rendering techniques to be applied to the mesh.
		 * @return The unique identifier (size_t) of the added mesh within the
		 * renderer, which can be used for future reference or manipulation of
		 * the mesh in the renderer.
		 */
		size_t addMesh(const utility::graphic::Mesh &mesh,
					   const std::string &materialName = "default_material",
					   const std::string &shader	   = "default");

		/**
		 * @brief Updates the state of the engine. This method is responsible
		 * for handling logic updates, input processing, and other non-rendering
		 * related tasks. It is typically called once per frame, allowing the
		 * engine to respond to user input, update the state of objects in the
		 * scene, and perform any necessary calculations or updates before
		 * rendering. The update method may also handle events such as
		 * collisions, physics simulations, or AI behavior, depending on the
		 * specific requirements of the application being developed with the
		 * engine.
		 *
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		void update();

		/**
		 * @brief Renders the current scene. This method is responsible for
		 * drawing the objects in the current scene to the screen using the
		 * renderer. It typically involves setting up the necessary graphics
		 * pipelines, binding resources, and issuing draw calls to render the
		 * objects in the scene. The render method may also handle
		 * post-processing effects, such as bloom or anti-aliasing, depending on
		 * the specific requirements of the application being developed with the
		 * engine.
		 *
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		void render();	  // For rendering the current scene.

		/**
		 * @brief Polls for events from the platform. This method is responsible
		 * for handling input events, window events, and other platform-specific
		 * events. It is typically called once per frame, allowing the engine to
		 * respond to user input, window resizing, and other events that may
		 * occur during the application's runtime. The pollEvents method
		 * interacts with the platform abstraction layer to retrieve and process
		 * events, ensuring that the engine can respond to user interactions and
		 * system events in a timely manner.
		 *
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 *
		 * @return A vector of shared pointers to Event objects representing the
		 * events that were polled from the platform. Each Event object contains
		 * information about the type of event, such as input events.
		 * The vector may contain events such as keyboard input, mouse input, or
		 * xr controller input, depending on the specific events that were
		 * polled from the platform during the current frame.
		 *
		 * see utility::event::Event for more details on the Event class and its
		 * derived classes representing specific types of events.
		 */
		std::vector<std::shared_ptr<utility::event::Event>> pollEvents();

		/**
		 * @brief Adds a new scene to the engine. This method allows users to
		 * add a new scene to the engine by providing the necessary data, such
		 * as texture paths and mesh data. The method takes in a vector of
		 * texture paths, which are used to load the textures for the scene, and
		 * a map of mesh data, which contains the information about the meshes
		 * to be rendered in the scene. The method creates a new Scene object
		 * using the provided data and adds it to the vector of scenes managed
		 * by the engine. This allows users to easily create and manage multiple
		 * scenes within the engine, enabling them to switch between different
		 * scenes as needed.
		 *
		 * @param sceneIndex Scene identifier.
		 *
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		void addScene(size_t sceneIndex);

		/**
		 * @brief Switches the current scene to the scene with the specified
		 * index. This method allows users to navigate between different scenes
		 * managed by the engine.
		 *
		 * @param sceneIndex The index of the scene to switch to.
		 */
		void switchScene(size_t sceneIndex);

		/**
		 * @brief Updates the scene with the specified index using new texture
		 * paths and mesh data. This method allows users to update an existing
		 * scene in the engine by providing new data, such as texture paths and
		 * mesh data.
		 *
		 * @param sceneIndex The index of the scene to update.
		 * @param texturePaths A vector of strings representing the file paths
		 * to the textures used in the scene. These textures may be associated
		 * with the materials used by the meshes.
		 * @param meshData A map where the key is a string representing the mesh
		 * name, and the value is a vector of GPUMesh objects representing the
		 * mesh data for that mesh name. Each GPUMesh object contains
		 * information about the vertices, indices, and material ID associated
		 * with that mesh.
		 */
		void updateScene(size_t sceneIndex,
						 std::vector<std::string> texturePaths,
						 std::map<std::string, std::vector<GPUMesh>> meshData);

		/**
		 * @brief Checks if the engine should capture viewport input. This
		 * method returns a boolean value indicating whether the engine is
		 * currently set to capture input events for the viewport, such as mouse
		 * movements or keyboard inputs.
		 *
		 * @return True if the engine should capture viewport input, false
		 * otherwise.
		 */
		bool shouldCaptureViewportInput(void) const
		{
			return _shouldCaptureViewportInput;
		}

		/**
		 * @brief Sets whether the engine should capture viewport input. This
		 * method allows users to enable or disable the capturing of input
		 * events for the viewport, such as mouse movements or keyboard inputs.
		 *
		 * @param shouldCapture A boolean value indicating whether the engine
		 * should capture viewport input (true) or not (false).
		 */
		void setShouldCaptureViewportInput(bool shouldCapture)
		{
			_shouldCaptureViewportInput = shouldCapture;
		}

		protected:
		/**
		 * The name of the engine.
		 */
		const std::string _engineName = "Evan Engine";

		/**
		 * The version of the engine as a string.
		 * This is a human-readable representation of the engine's version,
		 * typically following semantic versioning (e.g., "0.1.0").
		 * It can be used for display purposes or for logging the engine's
		 * version information.
		 */
		const std::string _engineVersion = "0.1.0";

		/**
		 * The version of the engine as a Version object.
		 * This is a structured representation of the engine's version,
		 * containing separate fields for the major, minor, and patch version
		 * numbers. The Version object allows for easier comparison and
		 * manipulation of version information, enabling features such as
		 * checking for compatibility or determining if an update is available
		 * based on version numbers.
		 */
		const std::shared_ptr<Version> _version =
			std::make_shared<Version>(0, 1, 0);

		/**
		 * A shared pointer to a DeviceContext object, which manages the Vulkan
		 * device and related resources. The DeviceContext is responsible for
		 * initializing Vulkan, selecting physical devices, creating logical
		 * devices, and managing command pools and queues.
		 */
		std::shared_ptr<DeviceContext> _deviceContext;

		/**
		 * A shared pointer to a Renderer object, which is responsible for
		 * rendering scenes. The Renderer manages graphics pipelines, descriptor
		 * sets, and other rendering resources needed to draw objects on the
		 * screen.
		 */
		std::shared_ptr<Renderer> _renderer;

		/**
		 * A map of scenes managed by the engine, where the key is a unique
		 * identifier (size_t) for each scene, and the value is a Scene object
		 * containing the data and resources for that scene. This allows the
		 * engine to manage multiple scenes simultaneously, enabling users to
		 * switch between different scenes as needed. Each Scene object contains
		 * the necessary data for rendering, such as meshes, materials, and
		 * textures, allowing the engine to efficiently manage and render
		 * multiple scenes within the application.
		 */
		std::map<size_t, std::shared_ptr<Scene>> _scenes;

		/**
		 * An index to keep track of the current scene being rendered or
		 * managed. This allows the engine to switch between different scenes as
		 * needed, enabling users to easily navigate through different parts of
		 * their application or game. The _currentScene index can be used to
		 * determine which scene is currently active and should be rendered or
		 * updated during the main loop of the engine.
		 */
		size_t _currentScene;

		/**
		 * A shared pointer to an ASwapchainContext object, which manages the
		 * Vulkan swapchain and related resources. The ASwapchainContext is
		 * responsible for creating and managing the swapchain, handling
		 * presentation, and managing synchronization primitives related to
		 * rendering and presentation. It provides an abstraction layer for
		 * managing the swapchain, allowing the engine to handle different
		 * platforms and rendering contexts without needing to directly interact
		 * with platform-specific details of swapchain management.
		 */
		std::shared_ptr<ASwapchainContext> _swapchainContext;

		/**
		 * A shared pointer to an IPlatform object, which provides an
		 * abstraction layer for platform-specific operations. The IPlatform
		 * interface defines methods for window management, input handling, and
		 * other platform-dependent functionality, allowing the engine to be
		 * portable across different operating systems and platforms.
		 */
		std::shared_ptr<IPlatform> _platform;

		/**
		 * A shared pointer to a RessourceProvider object, which is responsible
		 * for managing and loading various resources such as fonts, materials,
		 * and textures in a graphics application. The RessourceProvider
		 * provides methods to load resources from file paths or from asset
		 * objects, and it maintains internal maps to store loaded resources for
		 * efficient retrieval. It supports loading fonts, materials, and
		 * textures, and it can handle different shader types for materials.
		 */
		std::shared_ptr<utility::RessourceProvider> _ressourceProvider;

		/**
		 * A shared pointer to a RessourceManager object, which manages the
		 * loading and synchronization of GPU resources such as materials and
		 * textures. The RessourceManager interacts with the
		 * utility::RessourceProvider to load resources from disk or other
		 * sources, and creates corresponding GPU resources using the
		 * DeviceContext. It provides methods for synchronizing resources,
		 * retrieving specific materials or textures by ID, and managing the
		 * lifecycle of GPU resources to ensure efficient memory usage and
		 * performance in the rendering process.
		 */
		std::shared_ptr<RessourceManager> _ressourceManager;

		private:
		/**
		 * A counter to generate unique object IDs for scenes, render objects,
		 * or other entities managed by the engine. This counter is incremented
		 * each time a new object is created, ensuring that each object receives
		 * a unique identifier that can be used for tracking and management
		 * purposes within the engine. The _nextObjectID can be used to assign
		 * IDs to new scenes, render objects, or any other entities that require
		 * unique identification within the engine's data structures.
		 */
		size_t _nextObjectID = 1;

		/**
		 * When true, the engine copies keyboard and mouse input for movement:
		 * - Keyboard events are copied for entity movement and related actions.
		 * - Mouse movement is copied for view/camera rotation.
		 * - Event copying to UI entities is disabled.
		 *
		 * This flag is typically true when the UI does not have focus on
		 * any entity. When false, input is routed to the UI instead of the
		 * viewport.
		 */
		bool _shouldCaptureViewportInput = true;

		/**
		 * A vector of shared pointers to Event objects representing the events
		 * that have been captured for viewport input. This vector is used to
		 * store events such as keyboard input and mouse input that are relevant
		 * for controlling the camera or view in the engine. When
		 * _shouldCaptureViewportInput is true, relevant events are copied to
		 * this vector for processing in the handleViewportInput method,
		 * allowing the engine to respond to user input for camera movement and
		 * rotation while ensuring that UI entities do not receive input events
		 * when the viewport is focused.
		 */
		std::vector<std::shared_ptr<utility::event::Event>>
			_capturedViewportEvents;

		/**
		 * @brief Tracks whether the right mouse button is currently pressed.
		 *
		 * This state persists across frames to enable camera rotation when
		 * dragging with the right mouse button held down.
		 */
		bool _isRightMouseButtonPressed = false;

		/**
		 * @brief Stores the last mouse position for calculating delta movement.
		 *
		 * This is updated each frame when the right mouse button is pressed to
		 * calculate the mouse movement delta for camera rotation.
		 */
		utility::math::Vector2F _lastMousePosition { 0.0f, 0.0f };

		private:
		/**
		 * @brief Handles viewport input for camera movement and rotation.
		 *
		 * This method processes keyboard and mouse events to control the view:
		 * - WASD/Arrow keys for movement
		 * - Right mouse button + drag for rotation
		 * - Q/E for up/down movement
		 *
		 * @param events Vector of events to process.
		 */
		void handleViewportInput(
			const std::vector<std::shared_ptr<utility::event::Event>> &events);

		/**
		 * @brief Processes keyboard events for camera movement.
		 * @param keyboardEvent The keyboard event to process.
		 * @param orientation Current camera orientation (used for movement
		 * direction).
		 * @param position Current camera position (modified in place).
		 * @param movementSpeed Movement speed multiplier.
		 * @param deltaTime Time elapsed since the last frame (used for smooth
		 * movement).
		 */
		void handleKeyboardMovement(
			const std::shared_ptr<utility::event::KeyboardEvent> &keyboardEvent,
			const utility::graphic::OrientationF &orientation,
			utility::graphic::PositionF &position, float movementSpeed,
			float deltaTime);

		/**
		 * @brief Processes mouse button events for rotation state.
		 * @param mouseButtonEvent The mouse button event to process.
		 * @param isRightMouseButtonPressed Reference to right button state.
		 * @param lastMousePosition Reference to last mouse position.
		 */
		void handleMouseButtonEvent(
			const std::shared_ptr<utility::event::MouseButtonEvent>
				&mouseButtonEvent,
			bool &isRightMouseButtonPressed,
			utility::math::Vector2F &lastMousePosition);

		/**
		 * @brief Processes mouse motion events for camera rotation.
		 * @param mouseMotionEvent The mouse motion event to process.
		 * @param isRightMouseButtonPressed Current right button state.
		 * @param lastMousePosition Reference to last mouse position (updated).
		 * @param orientation Current camera orientation (modified in place).
		 * @param rotationSpeed Rotation speed multiplier.
		 */
		void handleMouseMotionEvent(
			const std::shared_ptr<utility::event::MouseMotionEvent>
				&mouseMotionEvent,
			bool isRightMouseButtonPressed,
			utility::math::Vector2F &lastMousePosition,
			utility::graphic::OrientationF &orientation, float rotationSpeed,
			float deltaTime);

		/**
		 * @brief Processes hand motion events for camera movement and rotation.
		 * @param handMotionEvent The hand motion event to process.
		 * @param position Current camera position (modified in place).
		 * @param orientation Current camera orientation (modified in place).
		 * @param movementSpeed Movement speed multiplier.
		 * @param rotationSpeed Rotation speed multiplier.
		 */
		void handleHandMotionEvent(
			const std::shared_ptr<utility::event::HandMotionEvent>
				&handMotionEvent,
			utility::graphic::PositionF &position,
			utility::graphic::OrientationF &orientation, float movementSpeed,
			float rotationSpeed, float deltaTime);

		/**
		 * @brief Tracks the last frame time for delta time calculation.
		 */
		std::chrono::steady_clock::time_point _lastFrameTime;

		/**
		 * @brief Stores the delta time from the last frame in seconds.
		 */
		float _deltaTime { 0.0f };

		/**
		 * @brief Get the delta time from the last frame in seconds.
		 * @return Delta time in seconds.
		 */
		float getDeltaTime(void) const;

		/**
		 * @brief Update the delta time based on current time.
		 * Called once per frame to update the delta time value.
		 */
		void updateDeltaTime(void);
	};
}	 // namespace evan
