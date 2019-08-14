#pragma once

#include <map>
#include <vulkan/vulkan.h>

namespace enumToChar
{
	const char *VkPhysicalDeviceType[] = {
		"VK_PHYSICAL_DEVICE_TYPE_OTHER",
		"VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
		"VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
		"VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
		"VK_PHYSICAL_DEVICE_TYPE_CPU",
	};
	
	std::map<VkResult, const char *> VkResult = {
		{VK_SUCCESS, "VK_SUCCESS"},
		{VK_NOT_READY, "VK_NOT_READY"},
		{VK_TIMEOUT, "VK_TIMEOUT"},
		{VK_EVENT_SET, "VK_EVENT_SET"},
		{VK_EVENT_RESET, "VK_EVENT_RESET"},
		{VK_INCOMPLETE, "VK_INCOMPLETE"},
		{VK_ERROR_OUT_OF_HOST_MEMORY, "VK_ERROR_OUT_OF_HOST_MEMORY"},
		{VK_ERROR_OUT_OF_DEVICE_MEMORY, "VK_ERROR_OUT_OF_DEVICE_MEMORY"},
		{VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED"},
		{VK_ERROR_DEVICE_LOST, "VK_ERROR_DEVICE_LOST"},
		{VK_ERROR_MEMORY_MAP_FAILED, "VK_ERROR_MEMORY_MAP_FAILED"},
		{VK_ERROR_LAYER_NOT_PRESENT, "VK_ERROR_LAYER_NOT_PRESENT"},
		{VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT"},
		{VK_ERROR_FEATURE_NOT_PRESENT, "VK_ERROR_FEATURE_NOT_PRESENT"},
		{VK_ERROR_INCOMPATIBLE_DRIVER, "VK_ERROR_INCOMPATIBLE_DRIVER"},
		{VK_ERROR_TOO_MANY_OBJECTS, "VK_ERROR_TOO_MANY_OBJECTS"},
	//VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
	//VK_ERROR_FRAGMENTED_POOL = -12,
	//VK_ERROR_OUT_OF_POOL_MEMORY = -1000069000,
	//VK_ERROR_INVALID_EXTERNAL_HANDLE = -1000072003,
	//VK_ERROR_SURFACE_LOST_KHR = -1000000000,
	//VK_ERROR_NATIVE_WINDOW_IN_USE_KHR = -1000000001,
	//VK_SUBOPTIMAL_KHR = 1000001003,
	//VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
	//VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
	//VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
	//VK_ERROR_INVALID_SHADER_NV = -1000012000,
	//VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT = -1000158000,
	//VK_ERROR_FRAGMENTATION_EXT = -1000161000,
	//VK_ERROR_NOT_PERMITTED_EXT = -1000174001,
	//VK_ERROR_INVALID_DEVICE_ADDRESS_EXT = -1000244000,
	//VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT = -1000255000,
	//VK_ERROR_OUT_OF_POOL_MEMORY_KHR = VK_ERROR_OUT_OF_POOL_MEMORY,
	//VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR = VK_ERROR_INVALID_EXTERNAL_HANDLE,
	//VK_RESULT_BEGIN_RANGE = VK_ERROR_FRAGMENTED_POOL,
	//VK_RESULT_END_RANGE = VK_INCOMPLETE,
	//VK_RESULT_RANGE_SIZE = (VK_INCOMPLETE - VK_ERROR_FRAGMENTED_POOL + 1),
	};
}