/*
** ETIB PROJECT, 2026
** evan
** File description:
** Engine
*/

#pragma once

#include "Version.hpp"

#include "DeviceContext.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "ASwapchainContext.hpp"
#include "IPlatform.hpp"

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
	class Engine
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
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		Engine(const std::shared_ptr<IPlatform> &platform);


		~Engine();

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
		 * @brief Polls for events from the platform. This method is responsible for
		 * handling input events, window events, and other platform-specific events.
		 * It is typically called once per frame, allowing the engine to
		 * respond to user input, window resizing, and other events that may occur during the
		 * application's runtime. The pollEvents method interacts with the platform abstraction
		 * layer to retrieve and process events, ensuring that the engine can
		 * respond to user interactions and system events in a timely manner.
		 *
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		void pollEvents();  // For handling input and other events.

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
		 * @note The Engine class is designed to be flexible and extensible,
		 * allowing for future enhancements and additions to the engine's
		 * capabilities. The current implementation focuses on establishing the
		 * core structure and functionality of the engine, with plans for
		 * further improvements and optimizations in the future.
		 */
		void addScene(size_t sceneIndex, std::vector<std::string> texturePaths,
					  std::map<std::string, std::vector<Mesh>> meshData);

		/**
		 * @brief Switches the current scene to the scene with the specified index.
		 * This method allows users to navigate between different scenes managed by the engine.
		 *
		 * @param sceneIndex The index of the scene to switch to.
		 */
		void switchScene(size_t sceneIndex);

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
		 * the necessary data for rendering, such as meshes, materials, and textures,
		 * allowing the engine to efficiently manage and render multiple
		 * scenes within the application.
		 */
		std::map<size_t, Scene> _scenes;

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

		private:
	};
}	 // namespace evan
