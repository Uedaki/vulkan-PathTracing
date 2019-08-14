#include "WindowApplication.h"

#include <stdexcept>

#include "LogMessage.h"
#include "VulkanEnumToChar.h"

#ifdef _DEBUG

namespace
{
	VkDebugUtilsMessengerEXT debugMessenger;

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			LOG_CRIT(pCallbackData->pMessage);
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			LOG_WARN(pCallbackData->pMessage);
		else
			LOG_MSG(pCallbackData->pMessage);
		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, 
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger, 
		const VkAllocationCallbacks *pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	void setupDebugUtilsMessenger(VkInstance instance)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}

#endif

WindowApplication::WindowApplication(int width, int height)
	: width(width), height(height)
{
	createGlfwWindow();
	initializeVulkan();
}

WindowApplication::~WindowApplication()
{
	if (currentBuffer)
		vkUnmapMemory(device, imageBuffersMemory[currentImage]);

	vkQueueWaitIdle(transferQueue);

	for (size_t i = 0; i < maxFrames; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphore[i], allocator);
		vkDestroySemaphore(device, renderFinishedSemaphore[i], allocator);
		vkDestroyFence(device, inFlightFences[i], allocator);
	}

	vkFreeCommandBuffers(device, commandPool, swapchainSize, commandBuffers);
	vkDestroyCommandPool(device, commandPool, allocator);

	if (imageBuffers && imageBuffersMemory)
	{
		for (uint32_t i = 0; i < swapchainSize; i++)
		{
			vkDestroyBuffer(device, imageBuffers[i], allocator);
			vkFreeMemory(device, imageBuffersMemory[i], allocator);
		}
		delete[] imageBuffersMemory;
		delete[] imageBuffers;
	}
	if (swapchainImage)
		delete[] swapchainImage;
	vkDestroySwapchainKHR(device, swapchain, allocator);
	vkDestroySurfaceKHR(instance, surface, allocator);
	vkDestroyDevice(device, allocator);
#ifdef _DEBUG
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocator);
#endif
	vkDestroyInstance(instance, allocator);
	glfwDestroyWindow(win);
	glfwTerminate();
}

bool WindowApplication::startFrame()
{
	glfwPollEvents();

	if (vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()) == VK_TIMEOUT)
		return (false);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &currentImage);
	return (true);
}

void *WindowApplication::getCurrentBuffer()
{
	VkDeviceSize size = static_cast<VkDeviceSize>(width * height) * 4;
	vkMapMemory(device, imageBuffersMemory[currentImage], 0, size, 0, &currentBuffer);
	return (currentBuffer);
}

void WindowApplication::render()
{
	if (currentBuffer)
	{

		vkUnmapMemory(device, imageBuffersMemory[currentImage]);
		currentBuffer = nullptr;
	}

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore[currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore[currentFrame];

	if (vkQueueSubmit(transferQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("Unable to transfer buffer for rendering.");

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentImage;

	vkQueuePresentKHR(transferQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % maxFrames;
}

bool WindowApplication::isWindowOpen()
{
	return (!glfwWindowShouldClose(win));
}

void WindowApplication::createGlfwWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	if (!(win = glfwCreateWindow(width, height, "Path tracing with Vulkan", nullptr, nullptr)))
		throw std::runtime_error("Failed to create GLFW window.");
}

void WindowApplication::initializeVulkan()
{
	createInstance();
#ifdef _DEBUG
	setupDebugUtilsMessenger(instance);
#endif
	selectPhysicalDevice();
	createSurface();
	selectQueueFamily();
	createLogicalDevice();
	createSwapchain();
	getSwapchainImages();
	createImageBuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

void WindowApplication::createInstance()
{
	uint32_t extensionsCount = 0;
	const char **extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
#ifdef  _DEBUG
	const char **tmp = new const char *[extensionsCount + 1];
	memcpy(tmp, extensions, extensionsCount * sizeof(const char *));
	tmp[extensionsCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	extensions = tmp;
	extensionsCount += 1;

	for (uint32_t i = 0; i < extensionsCount; i++)
	{
		LOG_MSG("Extension %s activated.", extensions[i]);
	}
#endif

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.enabledExtensionCount = extensionsCount;
	instanceInfo.ppEnabledExtensionNames = extensions;

#ifdef _DEBUG
	const char *validationLayer = "VK_LAYER_KHRONOS_validation";

	instanceInfo.enabledLayerCount = 1;
	instanceInfo.ppEnabledLayerNames = &validationLayer;
#endif

	if (vkCreateInstance(&instanceInfo, allocator, &instance) != VK_SUCCESS)
		throw std::runtime_error("Unable to create vulkan instance.");
}

void WindowApplication::selectPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	VkPhysicalDevice *physicalDevices = new VkPhysicalDevice[physicalDeviceCount];
	if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices) != VK_SUCCESS)
		throw std::runtime_error("No physical device available.");

	VkPhysicalDeviceProperties selectedPhysicalDeviceProperties;
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
		
		if (i == 0 || physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDevice = physicalDevices[i];
			selectedPhysicalDeviceProperties = physicalDeviceProperties;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("No physical device could be find.");
	delete[] physicalDevices;

	LOG_MSG("Physical device selected: %s.", selectedPhysicalDeviceProperties.deviceName);
}

void WindowApplication::selectQueueFamily()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	VkQueueFamilyProperties *queueFamilyProperties = new VkQueueFamilyProperties[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		VkBool32 isSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &isSupported);
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && isSupported)
		{
			transferQueueFamily = i;
			break;
		}
	}
	delete[] queueFamilyProperties;
}

void WindowApplication::createLogicalDevice()
{
	const char *deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	float deviceQueuePriority = 1.0f;
	VkDeviceQueueCreateInfo deviceQueueInfo = {};
	deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueInfo.queueFamilyIndex = transferQueueFamily;
	deviceQueueInfo.queueCount = 1;
	deviceQueueInfo.pQueuePriorities = &deviceQueuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &deviceQueueInfo;
	deviceInfo.enabledExtensionCount = 1;
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &deviceInfo, allocator, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create a logical device.");

	vkGetDeviceQueue(device, transferQueueFamily, 0, &transferQueue);
}

void WindowApplication::createSurface()
{
	VkResult res;
	if ((res = glfwCreateWindowSurface(instance, win, allocator, &surface)) != VK_SUCCESS)
		throw std::runtime_error("Failed to create surface.");
}

void WindowApplication::createSwapchain()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat();
	swapchainFormat = surfaceFormat.format;

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = surface;
	swapchainInfo.minImageCount = surfaceCapabilities.minImageCount;
	swapchainInfo.imageFormat = swapchainFormat;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 1;
	swapchainInfo.pQueueFamilyIndices = &transferQueueFamily;
	swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = selectPresentMode();
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &swapchainInfo, allocator, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("Failed to screate swap chain");
}

void WindowApplication::getSwapchainImages()
{
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainSize, nullptr);
	swapchainImage = new VkImage[swapchainSize];
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainSize, swapchainImage);
}

void WindowApplication::createImageBuffers()
{
	imageBuffers = new VkBuffer[swapchainSize];
	imageBuffersMemory = new VkDeviceMemory[swapchainSize];

	VkDeviceSize bufferSize = static_cast<VkDeviceSize>(width * height) * 4;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.pQueueFamilyIndices = &transferQueueFamily;

	for (uint32_t i = 0; i < swapchainSize; i++)
	{
		if (vkCreateBuffer(device, &bufferInfo, allocator, &imageBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the buffer.");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, imageBuffers[i], &memRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.memoryTypeIndex = selectMemoryType(memRequirements.memoryTypeBits, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocateInfo, allocator, &imageBuffersMemory[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate memory for the buffer.");

		if (vkBindBufferMemory(device, imageBuffers[i], imageBuffersMemory[i], 0) != VK_SUCCESS)
			throw std::runtime_error("Failed to bind buffer to its memory.");
	}
}

void WindowApplication::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = transferQueueFamily;

	if (vkCreateCommandPool(device, &commandPoolInfo, allocator, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool");
}

void WindowApplication::createCommandBuffers()
{
	commandBuffers = new VkCommandBuffer[swapchainSize];

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = commandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = swapchainSize;

	if (vkAllocateCommandBuffers(device, &commandBufferInfo, commandBuffers) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers.");

	for (uint32_t i = 0; i < swapchainSize; i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Unable to start recording command.");

		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchainImage[i];
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		VkBufferImageCopy imageCopy = {};
		imageCopy.bufferOffset = 0;
		imageCopy.bufferRowLength = 0;
		imageCopy.bufferImageHeight = 0;
		imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.imageSubresource.layerCount = 1;
		imageCopy.imageSubresource.mipLevel = 0;
		imageCopy.imageSubresource.baseArrayLayer = 0;
		imageCopy.imageExtent.width = static_cast<uint32_t>(width);
		imageCopy.imageExtent.height = static_cast<uint32_t>(height);
		imageCopy.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(commandBuffers[i], imageBuffers[i], swapchainImage[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = swapchainImage[i];
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			vkCmdPipelineBarrier(commandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Unable to close command recording.");
	}
}

void WindowApplication::createSyncObjects()
{
	if (maxFrames > swapchainSize)
		maxFrames = swapchainSize;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < maxFrames; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, allocator, &imageAvailableSemaphore[i]) != VK_SUCCESS
			|| vkCreateSemaphore(device, &semaphoreInfo, allocator, &renderFinishedSemaphore[i]) != VK_SUCCESS
			|| vkCreateFence(device, &fenceInfo, allocator, &inFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create synchronization objects.");
	}
}

VkSurfaceFormatKHR WindowApplication::selectSurfaceFormat()
{
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
	VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[surfaceFormatCount];
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats) != VK_SUCCESS)
		throw std::runtime_error("Unable to retreive available surface format.");

	VkSurfaceFormatKHR selectedSurfaceFormat = {};
	for (uint32_t i = 0; i < surfaceFormatCount; i++)
	{
		if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM
			&& surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			selectedSurfaceFormat = surfaceFormats[i];
			break;
		}
	}
	delete[] surfaceFormats;
	return (selectedSurfaceFormat);
}

VkPresentModeKHR WindowApplication::selectPresentMode()
{
	VkPresentModeKHR best;

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	VkPresentModeKHR *presentModes = new VkPresentModeKHR[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

	for (size_t i = 0; i < presentModeCount; i ++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentModes[i];
		else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			best = presentModes[i];
	}
	return (best);
}

uint32_t WindowApplication::selectMemoryType(uint32_t filter, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);
	for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
		if ((filter & (1 << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags)
		{
			return i;
		}
	}
	throw std::runtime_error("Failed to find suitable memory type");
}