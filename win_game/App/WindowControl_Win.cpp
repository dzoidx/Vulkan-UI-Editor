#include "WindowControl_Win.h"
#include <cassert>
#include "WindowControl.h"
#include <vulkan/vulkan_win32.h>

#if VULKAN

VkSurfaceKHR Win32WindowControl::CreateSurface(VkInstance& inst)
{
	VkWin32SurfaceCreateInfoKHR surfInfo = {};
	surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfInfo.hinstance = hinst_;
	surfInfo.hwnd = hwnd_;

	VkSurfaceKHR surface;
	VkResult result = vkCreateWin32SurfaceKHR(inst, &surfInfo, nullptr, &surface);
	assert(result == 0);

	return surface;
}

#endif
