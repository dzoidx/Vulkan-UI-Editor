#pragma once
#include "Windows.h"
#ifdef VULKAN
#include <vulkan/vulkan_core.h>
#endif
#include "WindowControl.h"
#include "Assets/Asset.h"

class Win32WindowControl : public WindowControl
{
public:
	Win32WindowControl() {};
	void Init(HINSTANCE hinst, HWND hwnd, int width, int height, uint32 dpi)
	{
		hinst_ = hinst;
		hwnd_ = hwnd;
		width_ = width;
		height_ = height;
		scale_ = dpi / 96.0f;
	}
#ifdef VULKAN
	virtual VkSurfaceKHR CreateSurface(VkInstance& inst);
#endif
	virtual int GetWidth() const { return width_; }
	virtual int GetHeight() const { return height_; }
	virtual float GetScale() const { return scale_; }
private:
	HINSTANCE hinst_;
	HWND hwnd_;
	int width_;
	int height_;
	float scale_;
};
