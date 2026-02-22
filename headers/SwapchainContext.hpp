/*
** EPITECH PROJECT, 2026
** evan
** File description:
** SwapchainContext
*/

#pragma once

#include "EvanPlatform.hpp"

#include <map>
#include <memory>

namespace evan
{
    class SwapchainImageContext {
        public:
            ~SwapchainImageContext();

            virtual void init() = 0;

        protected:
            void createDepthResources();

        private:
            std::vector<VkFence> _inFlightFences;
            std::vector<VkFence> _imagesInFlight;

            VkImage _depthImage;
            VkImageView _depthImageView;
            VkDeviceMemory _depthImageMemory;

            VkImage _colorImage;
            VkImageView _colorImageView;
            VkDeviceMemory _colorImageMemory;

            std::vector<VkImageView> _imageViews;
            VkExtent2D _extent;
            VkFormat _swapchainColorFormat = VK_FORMAT_UNDEFINED;
            std::vector<VkFramebuffer> _framebuffers;

            #ifdef OPENXR
            std::vector<XrSwapchainImageVulkan2KHR>  _swapchainImages;
            #elifdef GLFW
            std::vector<VkImage> _swapchainImages;
            #endif

            struct TextureImage {
                VkCommandPool _commandPool;
                VkQueue _graphicsQueue;            
            } _textureImage;
            
            VkSampleCountFlagBits _msaaSamples;
            VkDevice _logicalDevice;
            VkPhysicalDevice _physicalDevice;

    };

    /**
     * @class SwapchainContext
     * @brief Represents the context for managing the swapchain in the rendering system.
     *
     * The SwapchainContext class encapsulates the functionality related to managing the swapchain in the rendering system. It is responsible for handling the creation, configuration, and management of the swapchain, which is a series of images used for presenting rendered frames to the display. This class serves as a central point for managing swapchain-related operations in the engine.
     *
     */
    class SwapchainContext {
        public:
            ~SwapchainContext();

        protected:
            VkRenderPass _renderPass;

            #ifdef OPENXR
            std::map<XrSwapchian, std::shared_ptr<SwapchainImageContext>> _swapchainImages;
            #endif

        private:
            /**
             * @brief Creates the render pass for the swapchain context.
             * 
             * 
             */
            virtual void createRenderPass() = 0;
    };
} // namespace evan