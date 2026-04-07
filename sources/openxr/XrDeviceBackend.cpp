/*
** ETIB PROJECT, 2026
** evan
** File description:
** XrDeviceBackend
*/

#include "openxr/XrDeviceBackend.hpp"
#include "openxr/XrSwapchainContext.hpp"

#include <array>

evan::XrDeviceBackend::XrDeviceBackend(const IPlatform &platform)
{
	Version appVersion(0, 1, 0);
	createXrInstance(platform);
	getSystem();

	this->createInstance(platform, "Evan", appVersion);
	this->pickPhysicalDevice();
	this->createLogicalDevice();
	this->createSession();
	this->createVisualizedSpace();

	std::cout << _interactionProfile.getCurrentInteractionProfilePath(_XrInstance, _session,
												  "/user/hand/left")
			  << std::endl;
}

evan::XrDeviceBackend::~XrDeviceBackend()
{
	vkDeviceWaitIdle(_device);
	if (_device != VK_NULL_HANDLE) {
		vkDestroyDevice(_device, nullptr);
	}
	if (_session != XR_NULL_HANDLE) {
		xrDestroySession(_session);
	}
	vkDestroyInstance(_VkInstance, nullptr);
	if (_XrInstance != XR_NULL_HANDLE) {
		xrDestroyInstance(_XrInstance);
	}
}

////////////////////
// Public Methods //
////////////////////

bool evan::XrDeviceBackend::preprocessFrame(ASwapchainContext &swapchainContext)
{
	if (!_sessionRunning) {
		return false;
	}
	XrFrameState frameState { XR_TYPE_FRAME_STATE };
	XrFrameWaitInfo frameWaitInfo {
		.type = XR_TYPE_FRAME_WAIT_INFO,
	};
	XrResult result = xrWaitFrame(_session, &frameWaitInfo, &frameState);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to wait for OpenXR frame: " << result << std::endl;
		return false;
	}
	XrFrameBeginInfo frameBeginInfo { XR_TYPE_FRAME_BEGIN_INFO };
	result = xrBeginFrame(_session, &frameBeginInfo);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to begin OpenXR frame: " << result << std::endl;
		return false;
	}
	_predictedDisplayTime = frameState.predictedDisplayTime;
	if (frameState.shouldRender == XR_FALSE) {
		XrFrameEndInfo frameEndInfo { XR_TYPE_FRAME_END_INFO };
		frameEndInfo.displayTime		  = _predictedDisplayTime;
		frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
		frameEndInfo.layerCount			  = 0;
		frameEndInfo.layers				  = nullptr;
		xrEndFrame(_session, &frameEndInfo);
		return false;
	}
	XrViewState viewState { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	XrViewLocateInfo viewLocateInfo { XR_TYPE_VIEW_LOCATE_INFO };
	viewLocateInfo.viewConfigurationType =
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	viewLocateInfo.displayTime = _predictedDisplayTime;
	viewLocateInfo.space	   = _space;
	std::vector<XrView> &views =
		dynamic_cast<evan::XrSwapchainContext &>(swapchainContext)._views;
	XrResult locateResult = xrLocateViews(_session, &viewLocateInfo, &viewState,
										  static_cast<uint32_t>(views.size()),
										  &viewCount, views.data());
	if (locateResult != XR_SUCCESS) {
		std::cerr << "Failed to locate OpenXR views: " << locateResult
				  << std::endl;
		return false;
	}
	return true;
}

bool evan::XrDeviceBackend::processFrame(VkPresentInfoKHR presentInfo,
										 ASwapchainImage &swapchainImage)
{
	XrSwapchain swapchain =
		dynamic_cast<evan::XrSwapchainImage &>(swapchainImage)._swapchain;
	XrSwapchainImageReleaseInfo releaseInfo {
		XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO
	};
	XrResult result = xrReleaseSwapchainImage(swapchain, &releaseInfo);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to release OpenXR swapchain image: " << result
				  << std::endl;
		return false;
	}
	return true;
}

bool evan::XrDeviceBackend::postprocessFrame(
	ASwapchainContext &swapchainContext)
{
	dynamic_cast<evan::XrSwapchainContext &>(swapchainContext)
		.updateProjectionLayerViews();
	auto &projectionLayerViews =
		dynamic_cast<evan::XrSwapchainContext &>(swapchainContext)
			.getProjectionLayerViews();
	XrCompositionLayerProjection layer { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	layer.space		= _space;
	layer.viewCount = static_cast<uint32_t>(projectionLayerViews.size());
	layer.views		= projectionLayerViews.data();
	std::array<XrCompositionLayerBaseHeader *, 1> layers = {
		reinterpret_cast<XrCompositionLayerBaseHeader *>(&layer)
	};
	XrFrameEndInfo frameEndInfo { XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime		  = _predictedDisplayTime;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.layerCount			  = (layer.viewCount > 0) ? 1u : 0u;
	frameEndInfo.layers =
		(frameEndInfo.layerCount > 0) ? layers.data() : nullptr;

	XrResult result = xrEndFrame(_session, &frameEndInfo);
	if (result != XR_SUCCESS) {
		std::cerr << "Failed to end OpenXR frame: " << result << std::endl;
		return false;
	}
	return true;
}

evan::QueueFamilyIndices evan::XrDeviceBackend::findQueueFamilies()
{
	QueueFamilyIndices indices = {};
	uint32_t queueFamilyCount  = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount,
											 nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount,
											 queueFamilies.data());
	int i = 0;
	for (const auto &queueFamily: queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			break;
		}
		i++;
	}

	return indices;
}

uint32_t evan::XrDeviceBackend::countSwapchainFormats() const
{
	uint32_t swapchainFormatCount = 0;
	xrEnumerateSwapchainFormats(_session, 0, &swapchainFormatCount, nullptr);
	return swapchainFormatCount;
}

std::vector<int64_t> evan::XrDeviceBackend::enumerateSwapchainFormats(
	uint32_t swapchainFormatCount) const
{
	std::vector<int64_t> swapchainFormats(swapchainFormatCount);
	xrEnumerateSwapchainFormats(_session, swapchainFormatCount,
								&swapchainFormatCount, swapchainFormats.data());
	return swapchainFormats;
}

std::vector<XrViewConfigurationView>
	evan::XrDeviceBackend::enumerateViewConfigurations() const
{
	uint32_t viewConfigurationCount = 0;
	xrEnumerateViewConfigurationViews(_XrInstance, _systemId,
									  XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
									  0, &viewConfigurationCount, nullptr);

	if (viewConfigurationCount == 0) {
		std::cerr << "No view configurations found for the OpenXR system."
				  << std::endl;
		return {};
	}
	std::vector<XrViewConfigurationView> viewConfigurations(
		viewConfigurationCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	xrEnumerateViewConfigurationViews(
		_XrInstance, _systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		viewConfigurationCount, &viewConfigurationCount,
		viewConfigurations.data());
	return viewConfigurations;
}

///////////////////////
// Protected Methods //
///////////////////////

void evan::XrDeviceBackend::createInstance(const IPlatform &platform,
										   const std::string &appName,
										   Version &appVersion)
{
	XrGraphicsRequirementsVulkan2KHR graphicsRequirements {};
	PFN_xrGetVulkanGraphicsRequirements2KHR getVulkanGraphicsRequirements2KHR =
		nullptr;
	PFN_xrCreateVulkanInstanceKHR createVulkanInstanceKHR = nullptr;
	std::vector<const char *> extensions;
	std::vector<const char *> layers;

	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
	if (xrGetInstanceProcAddr(_XrInstance,
							  "xrGetVulkanGraphicsRequirements2KHR",
							  reinterpret_cast<PFN_xrVoidFunction *>(
								  &getVulkanGraphicsRequirements2KHR))
		!= XR_SUCCESS) {
		std::cerr << "Failed to get xrGetVulkanGraphicsRequirements2KHR "
					 "function pointer."
				  << std::endl;
		return;
	}
	XrResult xrResult = getVulkanGraphicsRequirements2KHR(
		_XrInstance, _systemId, &graphicsRequirements);
	if (xrResult != XR_SUCCESS) {
		std::cerr
			<< "Failed to query Vulkan graphics requirements through OpenXR."
			<< std::endl;
		return;
	}
	if (xrGetInstanceProcAddr(
			_XrInstance, "xrCreateVulkanInstanceKHR",
			reinterpret_cast<PFN_xrVoidFunction *>(&createVulkanInstanceKHR))
		!= XR_SUCCESS) {
		std::cerr << "Failed to get xrCreateVulkanInstanceKHR function pointer."
				  << std::endl;
		return;
	}

	VkApplicationInfo appInfo {};
	appInfo.sType			   = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = appName.c_str();
	appInfo.applicationVersion = appVersion.to_uint32_t();
	appInfo.pEngineName		   = "evan";
	appInfo.engineVersion	   = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion		   = graphicsRequirements.minApiVersionSupported;

	if (enableValidationLayers) {
		extensions = this->getRequiredInstanceExtensions();
		if (!extensions.empty()) {
			layers = this->getRequiredInstanceExtensionsAndroid();
		}
	}

	VkInstanceCreateInfo createInfo {};
	createInfo.sType				 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo		 = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount	   = static_cast<uint32_t>(layers.size());
	createInfo.ppEnabledLayerNames	   = layers.data();

	XrVulkanInstanceCreateInfoKHR vulkanCreateInfoKHR {};
	vulkanCreateInfoKHR.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
	vulkanCreateInfoKHR.vulkanCreateInfo	   = &createInfo;
	vulkanCreateInfoKHR.systemId			   = _systemId;
	vulkanCreateInfoKHR.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;

	VkResult result = VK_SUCCESS;
	createVulkanInstanceKHR(_XrInstance, &vulkanCreateInfoKHR, &_VkInstance,
							&result);
	if (result != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance through OpenXR."
				  << std::endl;
		return;
	}
}

void evan::XrDeviceBackend::createLogicalDevice()
{
	if (_physicalDevice == VK_NULL_HANDLE) {
		std::cerr << "Invalid Vulkan physical device provided." << std::endl;
		return;
	}

	PFN_xrCreateVulkanDeviceKHR createVulkanDeviceKHR = nullptr;

	if (xrGetInstanceProcAddr(
			_XrInstance, "xrCreateVulkanDeviceKHR",
			reinterpret_cast<PFN_xrVoidFunction *>(&createVulkanDeviceKHR))
		!= XR_SUCCESS) {
		std::cerr << "Failed to get xrCreateVulkanDeviceKHR function pointer."
				  << std::endl;
		return;
	}
	evan::QueueFamilyIndices indices = findQueueFamilies();
	if (!indices.graphicsFamily.has_value()) {
		std::cerr
			<< "Failed to find required queue families for the Vulkan device."
			<< std::endl;
		return;
	}

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount		 = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures {};

	VkDeviceCreateInfo createInfo {};
	createInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos	= &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures		= &deviceFeatures;

	XrVulkanDeviceCreateInfoKHR vulkanDeviceCreateInfoKHR {};
	vulkanDeviceCreateInfoKHR.type	   = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR;
	vulkanDeviceCreateInfoKHR.systemId = _systemId;
	vulkanDeviceCreateInfoKHR.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanDeviceCreateInfoKHR.vulkanCreateInfo		 = &createInfo;
	vulkanDeviceCreateInfoKHR.vulkanPhysicalDevice	 = _physicalDevice;

	VkResult result = VK_SUCCESS;
	createVulkanDeviceKHR(_XrInstance, &vulkanDeviceCreateInfoKHR, &_device,
						  &result);
	if (result != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan device through OpenXR."
				  << std::endl;
		return;
	}
}

void evan::XrDeviceBackend::pickPhysicalDevice()
{
	if (_VkInstance == VK_NULL_HANDLE) {
		std::cerr << "Invalid Vulkan instance provided." << std::endl;
		return;
	}
	PFN_xrGetVulkanGraphicsDevice2KHR getVulkanGraphicsDevice2KHR = nullptr;
	XrVulkanGraphicsDeviceGetInfoKHR deviceGetInfo {};

	if (xrGetInstanceProcAddr(_XrInstance, "xrGetVulkanGraphicsDevice2KHR",
							  reinterpret_cast<PFN_xrVoidFunction *>(
								  &getVulkanGraphicsDevice2KHR))
		!= XR_SUCCESS) {
		std::cerr
			<< "Failed to get xrGetVulkanGraphicsDevice2KHR function pointer."
			<< std::endl;
		return;
	}
	deviceGetInfo.type			 = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
	deviceGetInfo.systemId		 = _systemId;
	deviceGetInfo.vulkanInstance = _VkInstance;

	if (getVulkanGraphicsDevice2KHR(_XrInstance, &deviceGetInfo,
									&_physicalDevice)
		!= XR_SUCCESS) {
		std::cerr << "Failed to get Vulkan physical device through OpenXR."
				  << std::endl;
		return;
	}
	return;
}

void evan::XrDeviceBackend::createVisualizedSpace()
{
	XrReferenceSpaceCreateInfo spaceCreateInfo {
		XR_TYPE_REFERENCE_SPACE_CREATE_INFO
	};
	spaceCreateInfo.referenceSpaceType	 = XR_REFERENCE_SPACE_TYPE_STAGE;
	spaceCreateInfo.poseInReferenceSpace = { { 0, 0, 0, 1 }, { 0, 0, 0 } };

	XrResult result =
		xrCreateReferenceSpace(_session, &spaceCreateInfo, &_space);
	if (result != XR_SUCCESS) {
		spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
		result = xrCreateReferenceSpace(_session, &spaceCreateInfo, &_space);
		if (result != XR_SUCCESS) {
			std::cerr << "Failed to create OpenXR reference space: " << result
					  << std::endl;
			return;
		}
	}
}

std::vector<VkExtensionProperties>
	evan::XrDeviceBackend::getInstanceExtensions() const
{
	uint32_t extensionCount = 0;
	std::vector<VkExtensionProperties> extensions;
	std::string layerName = "VK_LAYER_KHRONOS_validation";

	if (vkEnumerateInstanceExtensionProperties(layerName.c_str(),
											   &extensionCount, nullptr)
		!= VK_SUCCESS) {
		std::cerr << "Failed to enumerate OpenXR instance extension properties."
				  << std::endl;
		return extensions;
	}

	extensions.resize(extensionCount);
	if (vkEnumerateInstanceExtensionProperties(
			layerName.c_str(), &extensionCount, extensions.data())
		!= VK_SUCCESS) {
		std::cerr << "Failed to enumerate OpenXR instance extension properties."
				  << std::endl;
		return {};
	}

	return extensions;
}

/////////////////////
// Private Methods //
/////////////////////

void evan::XrDeviceBackend::createXrInstance(const IPlatform &platform)
{
	std::vector<std::string> requiredExtensions =
		platform.getRequiredInstanceExtensions();
	std::vector<const char *> extensions;
	extensions.reserve(requiredExtensions.size());

	for (const auto &ext: requiredExtensions) {
		extensions.push_back(ext.c_str());
	}

	XrInstanceCreateInfo createInfo { XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.enabledExtensionNames = extensions.data();
	createInfo.enabledApiLayerCount	 = 0;
	createInfo.enabledApiLayerNames	 = nullptr;
	std::strcpy(createInfo.applicationInfo.applicationName, "evan");
	createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

	auto xrPlatform = dynamic_cast<const IXrPlatform *>(&platform);

	createInfo.next = xrPlatform->getInstanceCreateInfo();

	XrResult result = xrCreateInstance(&createInfo, &_XrInstance);
	if (result != XR_SUCCESS) {
		// TODO: Throw an exception or handle the error appropriately
		std::cerr << "Failed to create OpenXR instance: " << result
				  << std::endl;
		return;
	}
}

void evan::XrDeviceBackend::getSystem()
{
	XrSystemGetInfo systemInfo { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	XrResult result = xrGetSystem(_XrInstance, &systemInfo, &_systemId);
	if (result != XR_SUCCESS) {
		// TODO: Throw an exception or handle the error appropriately
		std::cerr << "Failed to get OpenXR system: " << result << std::endl;
		return;
	}
}

void evan::XrDeviceBackend::createSession()
{
	XrSessionCreateInfo sessionCreateInfo			  = {};
	XrGraphicsBindingVulkan2KHR graphicsBindingVulkan = {};
	QueueFamilyIndices indices						  = findQueueFamilies();

	if (indices.graphicsFamily == std::nullopt) {
		std::cerr << "Failed to create OpenXR session: no graphics queue "
					 "family found."
				  << std::endl;
		return;
	}

	graphicsBindingVulkan.type			 = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
	graphicsBindingVulkan.instance		 = _VkInstance;
	graphicsBindingVulkan.physicalDevice = _physicalDevice;
	graphicsBindingVulkan.device		 = _device;
	graphicsBindingVulkan.queueFamilyIndex = indices.graphicsFamily.value();

	sessionCreateInfo.type	   = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.next	   = &graphicsBindingVulkan;
	sessionCreateInfo.systemId = _systemId;

	if (xrCreateSession(_XrInstance, &sessionCreateInfo, &_session)
		!= XR_SUCCESS) {
		std::cerr << "Failed to create OpenXR session." << std::endl;
		return;
	}
}

std::vector<const char *>
	evan::XrDeviceBackend::getRequiredInstanceExtensions() const
{
	auto availableExtensions = getInstanceExtensions();
	std::vector<const char *> requiredExtensions;

	for (const auto &ext: availableExtensions) {
		if (std::strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			== 0) {
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		} else if (std::strcmp(ext.extensionName,
							   VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)
				   == 0) {
			requiredExtensions.push_back(
				VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
		}
	}
	return requiredExtensions;
}

std::vector<const char *>
	evan::XrDeviceBackend::getRequiredInstanceExtensionsAndroid()
{
	std::vector<const char *> availableLayers;

	for (const auto &layer: this->getAvailableLayers()) {
		if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
			availableLayers.emplace_back("VK_LAYER_KHRONOS_validation");
		}
	}
	return availableLayers;
}
