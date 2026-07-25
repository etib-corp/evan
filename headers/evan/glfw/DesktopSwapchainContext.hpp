/*
** ETIB PROJECT, 2026
** evan
** File description:
** DesktopSwapchainContext
*/

#pragma once

#include "evan/ASwapchainContext.hpp"

#include "evan/glfw/DesktopSwapchainImage.hpp"

#include <memory>

namespace evan
{
	class DeviceContext;

	/**
	 * @brief Swapchain context for desktop platforms using GLFW.
	 *
	 * This class manages the swapchain for desktop applications, providing
	 * functionality to acquire images, get projection and view matrices, and
	 * handle synchronization. It inherits from ASwapchainContext and implements
	 * the necessary virtual functions for desktop-specific behavior.
	 *
	 * The constructor initializes the swapchain context with the given device
	 * context and GLFW window. The destructor ensures proper cleanup of
	 * resources. The aquireImage function allows acquiring an image from the
	 * swapchain, while getProjection and getView provide the necessary matrices
	 * for rendering.
	 *
	 * @note This class is designed to work with Vulkan and GLFW, and it assumes
	 * that the necessary Vulkan setup has been performed before its
	 * instantiation. It also relies on the DeviceContext for managing Vulkan
	 * resources and synchronization.
	 */
	class DesktopSwapchainContext: public ASwapchainContext
	{
		public:
		/**
		 * @brief Constructs a DesktopSwapchainContext instance with the
		 * provided device context and GLFW window.
		 *
		 * This constructor initializes the swapchain context for desktop
		 * platforms using GLFW. It takes a reference to a DeviceContext
		 * instance, which provides access to Vulkan resources and
		 * synchronization mechanisms, and a pointer to a GLFWwindow, which is
		 * used to create the swapchain and manage the rendering surface.
		 *
		 * @param deviceContext A reference to the DeviceContext instance that
		 * provides access to Vulkan resources and synchronization mechanisms.
		 * @param window A pointer to the GLFWwindow that is used to create the
		 * swapchain and manage the rendering surface for desktop applications.
		 *
		 * @note The constructor performs necessary initialization steps to set
		 * up the swapchain context, including creating the swapchain,
		 * configuring synchronization primitives, and preparing for rendering
		 * operations. It is essential to ensure that the Vulkan instance and
		 * logical device have been properly initialized before creating an
		 * instance of DesktopSwapchainContext.
		 */
		DesktopSwapchainContext(const DeviceContext &deviceContext,
								GLFWwindow *window);

		~DesktopSwapchainContext();

		/**
		 * @brief Destroys the swapchain and releases associated resources.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to handle the destruction of the swapchain and the
		 * release of any resources associated with it. It takes a Vulkan device
		 * as a parameter, which is used to perform the necessary cleanup
		 * operations. Implement this function to ensure that all resources
		 * related to the swapchain are properly released when the swapchain is
		 * no longer needed.
		 *
		 * @param device The Vulkan device used to perform cleanup operations
		 * for the swapchain.
		 *
		 * @note This function is essential for managing the lifecycle of the
		 * swapchain and ensuring that resources are properly released to
		 * prevent memory leaks and other issues in the rendering system. It
		 * should be implemented to handle the specific cleanup logic for the
		 * swapchain in the context of desktop applications using GLFW.
		 */
		void destroy(VkDevice device) override;

		/**
		 * @brief Recreates the swapchain and associated resources.
		 *
		 * This function is responsible for recreating the swapchain and any
		 * associated resources when the swapchain becomes out of date or needs
		 * to be recreated due to changes in the window size or other factors.
		 * It takes a reference to the DeviceContext instance, which provides
		 * access to Vulkan resources and synchronization mechanisms. Implement
		 * this function to ensure that the swapchain is properly recreated and
		 * that all necessary resources are reinitialized for rendering
		 * operations.
		 *
		 * @param deviceContext A reference to the DeviceContext instance that
		 * provides access to Vulkan resources and synchronization mechanisms.
		 * @param renderpass The Vulkan render pass that will be used with the
		 * swapchain images during rendering operations.
		 */
		void recreateSwapchain(const DeviceContext &deviceContext,
							   VkRenderPass renderpass) override;

		/**
		 * @brief Acquires the next available image from the swapchain for
		 * rendering.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to handle the acquisition of the next available
		 * image from the swapchain for rendering. It takes several parameters,
		 * including the index of the image to acquire, the Vulkan device, a
		 * semaphore to signal when the image is available, a fence to signal
		 * when the image is in flight, and a reference to store the index of
		 * the acquired image. Implement this function to ensure that the
		 * rendering system can acquire images from the swapchain for rendering
		 * operations.
		 *
		 * @param index The index of the image to acquire from the swapchain.
		 * @param device The Vulkan device used to perform the image
		 * acquisition.
		 * @param imageAvailableSemaphore A Vulkan semaphore that will be
		 * signaled when the image is available for rendering.
		 * @param inFlightFence A Vulkan fence that will be signaled when the
		 * image is in flight and ready for rendering operations.
		 * @param imageIndex A reference to a uint32_t variable where the index
		 * of the acquired image will be stored.
		 *
		 * @return VkResult The result of the image acquisition operation,
		 * indicating success or failure. Implement this function to return
		 * appropriate Vulkan result codes based on the outcome of the image
		 * acquisition process.
		 *
		 * @note This function is crucial for managing the rendering workflow,
		 * as it allows the application to acquire images from the swapchain for
		 * rendering operations. Proper implementation of this function ensures
		 * that synchronization is handled correctly and that the rendering
		 * system can efficiently acquire and use images from the swapchain.
		 */
		VkResult aquireImage(uint32_t index, VkDevice device,
							 VkSemaphore imageAvailableSemaphore,
							 VkFence inFlightFence,
							 uint32_t &imageIndex) override;

		/**
		 * @brief Waits for the specified image to be available for rendering.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to handle waiting for a specific image to be
		 * available for rendering. In the context of desktop applications using
		 * GLFW, this function may not require any specific implementation, as
		 * synchronization is typically handled through Vulkan semaphores and
		 * fences. However, it is essential to provide an implementation to
		 * satisfy the interface requirements of ASwapchainContext.
		 *
		 * @param index The index of the image to wait for.
		 *
		 * @note This function does not perform any specific operations in the
		 * context of desktop applications using GLFW, as synchronization is
		 * managed through Vulkan primitives. However, it is implemented to
		 * fulfill the interface requirements of ASwapchainContext and to
		 * provide a placeholder for any potential future synchronization logic
		 * that may be needed for specific use cases or platform-specific
		 * behavior.
		 */
		void waitForImage(uint32_t index) override
		{
		}

		/**
		 * @brief Retrieves the projection matrix for the specified index.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to provide the projection matrix for the specified
		 * index. The projection matrix is essential for rendering operations,
		 * as it defines how 3D objects are projected onto the 2D screen.
		 * Implement this function to return the appropriate projection matrix
		 * based on the rendering context and the specific requirements of the
		 * application.
		 *
		 * @param index The index for which to retrieve the projection matrix.
		 *
		 * @return glm::mat4 The projection matrix corresponding to the
		 * specified index. Implement this function to return the appropriate
		 * projection matrix based on the rendering context and the specific
		 * requirements of the application, such as the field of view, aspect
		 * ratio, and near/far clipping planes.
		 *
		 * @note This function is crucial for configuring the graphics pipeline
		 * and ensuring that rendering operations are performed correctly.
		 * Proper implementation of this function allows the application to
		 * provide the necessary projection matrix for rendering 3D objects in
		 * the scene.
		 */
		glm::mat4 getProjection(std::size_t index) const override;

		/**
		 * @brief Retrieves the view matrix for the specified index.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to provide the view matrix for the specified index.
		 * The view matrix is essential for rendering operations, as it defines
		 * the position and orientation of the camera in the 3D scene. Implement
		 * this function to return the appropriate view matrix based on the
		 * rendering context and the specific requirements of the application.
		 *
		 * @param index The index for which to retrieve the view matrix.
		 *
		 * @return glm::mat4 The view matrix corresponding to the specified
		 * index. Implement this function to return the appropriate view matrix
		 * based on the rendering context and the specific requirements of the
		 * application, such as the camera position, target, and up vector.
		 *
		 * @note This function is crucial for configuring the graphics pipeline
		 * and ensuring that rendering operations are performed correctly.
		 * Proper implementation of this function allows the application to
		 * provide the necessary view matrix for rendering 3D objects in the
		 * scene, enabling the correct positioning and orientation of the
		 * camera.
		 */
		utility::graphic::ViewF getView(std::size_t index) const override;

		/**
		 * @brief Sets the view matrix for the specified image index in the
		 * swapchain.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to set the view matrix associated with the
		 * specified image index in the swapchain. The view matrix is used in
		 * rendering operations to transform world coordinates into view space
		 * for the corresponding image.
		 *
		 * @param index The index of the image in the swapchain for which to
		 * set the view matrix.
		 * @param view The view matrix as a glm::mat4 to set for the specified
		 * image index in the swapchain.
		 */
		void setView(std::size_t index,
					 const utility::graphic::ViewF &view) override;

		/**
		 * @brief Retrieves the number of views in the swapchain context.
		 *
		 * This function overrides the pure virtual function from
		 * ASwapchainContext to return the number of views associated with the
		 * swapchain context. The number of views is essential for configuring
		 * the rendering pipeline and ensuring that rendering operations are
		 * performed correctly for each view.
		 *
		 * @return The number of views in the swapchain context.
		 */
		std::size_t getViewCount(void) const override;

		private:
		/**
		 * @brief View state for the swapchain context
		 */
		utility::graphic::ViewF _view;

		/**
		 * @brief GLFW reference window for the swapchain context
		 */
		GLFWwindow *_referenceWindow;
	};
}	 // namespace evan
