#pragma once
#ifdef VULKAN
#include <vulkan/vulkan_core.h>
#endif

class WindowControl
{
protected:
	WindowControl() {}
	~WindowControl() {}
public:
#ifdef VULKAN
	virtual VkSurfaceKHR CreateSurface(VkInstance& inst) = 0;
#endif
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual float GetScale() const = 0;
};
