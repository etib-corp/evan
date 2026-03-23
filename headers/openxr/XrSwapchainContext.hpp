/*
** EPITECH PROJECT, 2026
** evan
** File description:
** XrSwapchainContext
*/

#pragma once

#include "ASwapchainContext.hpp"
#include "ASwapchainImage.hpp"
#include "DeviceContext.hpp"
#include "openxr/XrDeviceBackend.hpp"
#include "openxr/XrSwapchainImage.hpp"

#include <map>
#include <memory>

namespace evan
{
	/**
	 * @brief The XrSwapchainContext class manages the swapchain context for an
	 * OpenXR application using Vulkan. It inherits from ASwapchainContext and
	 * provides implementations for acquiring images from the swapchain, waiting
	 * for images to be available, and updating projection layer views based on
	 * the current view configurations. The class also maintains a vector of
	 * XrCompositionLayerProjectionView structures that represent the projection
	 * layer views for each view configuration, allowing for efficient
	 * management of rendering resources in a VR application.
	 *
	 * @note The XrSwapchainContext class is designed to work with the
	 * DeviceContext class to manage Vulkan resources and ensure proper
	 * synchronization when acquiring images from the swapchain. It also
	 * provides methods to retrieve projection and view matrices for rendering
	 * operations, making it an essential component of the rendering system in
	 * an OpenXR application.
	 */
	class XrSwapchainContext: public ASwapchainContext
	{
		public:
		/**
		 * @brief Constructs an XrSwapchainContext instance using the provided
		 * DeviceContext.
		 *
		 * This constructor initializes the XrSwapchainContext by setting up the
		 * necessary Vulkan resources and configurations based on the provided
		 * DeviceContext. It may involve querying the swapchain support details,
		 * creating projection layer views, and preparing for image acquisition
		 * from the swapchain. Implement this constructor to ensure that the
		 * XrSwapchainContext is properly initialized and ready for use in
		 * rendering operations within an OpenXR application.
		 *
		 * @param deviceContext A reference to a DeviceContext instance that
		 * provides access to Vulkan resources and configurations needed for
		 * initializing the XrSwapchainContext. This context is essential for
		 * managing the swapchain and ensuring proper synchronization when
		 * acquiring images for rendering.
		 *
		 * @note The constructor should handle any necessary setup for the
		 * swapchain context, including querying swapchain support details,
		 * creating projection layer views based on the current view
		 * configurations, and preparing for image acquisition from the
		 * swapchain. Ensure that all Vulkan resources are properly initialized
		 * and that the XrSwapchainContext is ready for use in rendering
		 * operations within an OpenXR application.
		 */
		XrSwapchainContext(const DeviceContext &deviceContext);

		/**
		 * @brief Destroys the XrSwapchainContext and releases associated
		 * resources.
		 *
		 * This destructor is responsible for cleaning up any resources
		 * associated with the XrSwapchainContext, such as Vulkan resources,
		 * projection layer views, and any other resources that were allocated
		 * during the lifetime of the context. Implement this destructor to
		 * ensure that all resources are properly released when the
		 * XrSwapchainContext is no longer needed, preventing memory leaks and
		 * ensuring efficient resource management in the application.
		 *
		 * @note The destructor should handle the cleanup of all resources
		 * associated with the XrSwapchainContext, including Vulkan resources,
		 * projection layer views, and any other resources that were allocated
		 * during the lifetime of the context. Ensure that all resources are
		 * properly released to prevent memory leaks and ensure efficient
		 * resource management in the application.
		 */
		void destroy(VkDevice device) override;

		/**
		 * @brief Acquires an image from the swapchain for rendering.
		 *
		 * This method is responsible for acquiring an image from the swapchain,
		 * which can then be used for rendering operations. It takes the index
		 * of the swapchain image to acquire, the Vulkan device, a semaphore to
		 * signal when the image is available, a fence to ensure
		 * synchronization, and a reference to store the acquired image index.
		 * Implement this method to properly acquire images from the swapchain
		 * and ensure synchronization with rendering operations in an OpenXR
		 * application.
		 *
		 * @param index The index of the swapchain image to acquire.
		 * @param device The Vulkan device used for acquiring the image.
		 * @param imageAvailableSemaphore A Vulkan semaphore that will be
		 * signaled when the image is available for rendering.
		 * @param inFlightFence A Vulkan fence used to ensure synchronization
		 * when acquiring the image.
		 * @param imageIndex A reference to store the index of the acquired
		 * image from the swapchain.
		 *
		 * @return VkResult The result of the image acquisition operation,
		 * indicating success or failure. Implement this method to return
		 * appropriate Vulkan result codes based on the outcome of the image
		 * acquisition process.
		 */
		VkResult aquireImage(uint32_t index, VkDevice device,
							 VkSemaphore imageAvailableSemaphore,
							 VkFence inFlightFence,
							 uint32_t &imageIndex) override;

		/**
		 * @brief Waits for an image from the swapchain to become available for
		 * rendering.
		 *
		 * This method is responsible for waiting until an image from the
		 * swapchain is available for rendering. It takes the index of the
		 * swapchain image to wait for and ensures that the application does not
		 * proceed with rendering operations until the specified image is ready.
		 * Implement this method to properly synchronize rendering operations
		 * with the availability of swapchain images in an OpenXR application.
		 *
		 * @param index The index of the swapchain image to wait for.
		 *
		 * @return void This method does not return a value, but it should
		 * ensure that the application waits until the specified image from the
		 * swapchain is available for rendering before proceeding with any
		 * rendering operations.
		 */
		void waitForImage(uint32_t index) override;

		/**
		 * @brief Updates the projection layer views based on the current view
		 * configurations.
		 *
		 * This method is responsible for updating the projection layer views
		 * based on the current view configurations, which may change due to
		 * user interactions or changes in the VR environment. It should ensure
		 * that the projection layer views are properly updated to reflect any
		 * changes in the view configurations, allowing for accurate rendering
		 * of the scene in an OpenXR application.
		 *
		 * @return void This method does not return a value, but it should
		 * ensure that the projection layer views are updated based on the
		 * current view configurations, allowing for accurate rendering of the
		 * scene in an OpenXR application.
		 */
		void updateProjectionLayerViews();

		/**
		 * @brief Retrieves the projection layer views for the current view
		 * configurations.
		 *
		 * This method returns a reference to a vector of
		 * XrCompositionLayerProjectionView structures that represent the
		 * projection layer views for the current view configurations. These
		 * views are essential for rendering the scene accurately in an OpenXR
		 * application, as they provide the necessary information for projecting
		 * the scene onto the display. Implement this method to return the
		 * appropriate projection layer views based on the current view
		 * configurations.
		 *
		 * @return const std::vector<XrCompositionLayerProjectionView>& A
		 * reference to a vector of XrCompositionLayerProjectionView structures
		 * that represent the projection layer views for the current view
		 * configurations. This vector should be updated based on any changes in
		 * the view configurations to ensure accurate rendering of the scene in
		 * an OpenXR application.
		 */
		const std::vector<XrCompositionLayerProjectionView> &
			getProjectionLayerViews() const;

		/**
		 * @brief Retrieves the projection matrix for a specific view index.
		 *
		 * This method returns the projection matrix for a specific view index,
		 * which is essential for rendering the scene accurately in an OpenXR
		 * application. The projection matrix is used to project the 3D scene
		 * onto the 2D display, and it may vary based on the view configurations
		 * and user interactions. Implement this method to return the
		 * appropriate projection matrix for the specified view index.
		 *
		 * @param index The index of the view for which to retrieve the
		 * projection matrix.
		 *
		 * @return glm::mat4 The projection matrix for the specified view index,
		 * which should be calculated based on the current view configurations
		 * and any changes in the VR environment to ensure accurate rendering of
		 * the scene in an OpenXR application.
		 */
		glm::mat4 getProjection(int index) const override;

		/**
		 * @brief Retrieves the view matrix for a specific view index.
		 *
		 * This method returns the view matrix for a specific view index, which
		 * is essential for rendering the scene accurately in an OpenXR
		 * application. The view matrix is used to transform the 3D scene based
		 * on the position and orientation of the user's head and controllers,
		 * and it may vary based on user interactions and changes in the VR
		 * environment. Implement this method to return the appropriate view
		 * matrix for the specified view index.
		 *
		 * @param index The index of the view for which to retrieve the view
		 * matrix.
		 *
		 * @return glm::mat4 The view matrix for the specified view index, which
		 * should be calculated based on the current view configurations, user
		 * interactions, and any changes in the VR environment to ensure
		 * accurate rendering of the scene in an OpenXR application.
		 */
		glm::mat4 getView(int index) const override;

		/**
		 * Vector of XrView structures for each view configuration.
		 */
		std::vector<XrView> _views;

		/**
		 * Vector of XrViewConfigurationView structures for each view
		 * configuration, containing details about the recommended image rect
		 * dimensions and swapchain sample counts for each view.
		 */
		std::vector<XrViewConfigurationView> _viewsConfigurations;

		private:
		/**
		 * Vector of XrCompositionLayerProjectionView structures for each
		 * projection layer view.
		 */
		std::vector<XrCompositionLayerProjectionView> _projectionLayerViews;
	};
}	 // namespace evan