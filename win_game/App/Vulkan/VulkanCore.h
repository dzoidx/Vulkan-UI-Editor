// (c) 2021 Alexander Morgan (dzoidx@gmail.com). All rights reserved.
#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "Types.h"
#include "App/WindowControl.h"
#include "Assets/TextureAsset.h"
#include "UI/UINode.h"

class VulkanCore;
class VulkanApp;

struct VulkanDeviceAllocation
{
	uint64 Size;
	VkDeviceMemory Memory;
};

enum class VulkanCommandPoolType
{
	Graphics,
	Transfer,
	Present
};

struct InstantCommandBuffer
{
	VkCommandBuffer Buffer;
	VulkanCommandPoolType Type;
};

class VulkanBuffer
{
public:
	VulkanBuffer(VulkanCore* core) : core_(core), buffer_(VK_NULL_HANDLE), memory_(VK_NULL_HANDLE), hostCoherent_(false) {}
	~VulkanBuffer();

	void Init(VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkDeviceSize size, void* data = nullptr);
	void Update(void* data, VkDeviceSize offset, VkDeviceSize size);
protected:
	void FreeResources();
protected:
	VulkanCore* core_;
	VkBuffer buffer_;
	VkDeviceMemory memory_;
	bool hostCoherent_;
};

class VulkanUIVertexBuffer : VulkanBuffer
{
public:
	VulkanUIVertexBuffer(VulkanCore* core) : VulkanBuffer(core) {}

	void Init(UIInputLayout* data, uint32 count);
	void Update(UIInputLayout* data, uint32 index, uint32 count);

	VkBuffer* GetBuffer() { return &buffer_; }
};

class VulkanIndexBuffer : VulkanBuffer
{
public:
	VulkanIndexBuffer(VulkanCore* core) : VulkanBuffer(core) {}

	void Init(uint32* data, uint32 count);
	void Update(uint32* data, uint32 index, uint32 count);

	VkBuffer GetBuffer() { return buffer_; }
};

struct VulkanTexture
{
	VkImage Image;
	VkImageView View;
	VkFormat Format;
	VulkanDeviceAllocation AllocInfo;
};

//TODO: освободить ресурсы текстуры правильно
class VulkanTextureBuffer : VulkanBuffer
{
public:
	VulkanTextureBuffer(VulkanCore* core, VulkanApp* app) : VulkanBuffer(core), app_(app), texture_(nullptr) {}
	~VulkanTextureBuffer() { FreeResources(); }
	void Init(TextureAsset& data);
	VkBuffer GetBuffer() { return buffer_; }
	void FreeResources();

	VulkanTexture TextureInfo;
private:
	VulkanApp* app_;
	TextureAsset* texture_;
};

struct VulkanPhysicalDevices {
	std::vector<VkPhysicalDevice> m_devices;
	std::vector<VkPhysicalDeviceProperties> m_devProps;
	std::vector<VkPhysicalDeviceFeatures> m_devFeatures;
	std::vector<VkPhysicalDeviceMemoryProperties> m_devMemProps;
	std::vector< std::vector<VkQueueFamilyProperties> > m_qFamilyProps;
	std::vector< std::vector<VkBool32> > m_qSupportsPresent;
	std::vector< std::vector<VkSurfaceFormatKHR> > m_surfaceFormats;
	std::vector<VkSurfaceCapabilitiesKHR> m_surfaceCaps;
};

class VulkanCore
{
public:
	VulkanCore(const char* pAppName) : m_appName(pAppName) {};
	~VulkanCore() {};

	bool Init(WindowControl* pWindowControl);

	const VkPhysicalDevice& GetPhysDevice() const { return m_physDevices.m_devices[m_gfxDevIndex]; }

	const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_physDevices.m_surfaceFormats[m_gfxDevIndex][0]; }
	const VkSurfaceCapabilitiesKHR GetSurfaceCaps() const { return m_physDevices.m_surfaceCaps[m_gfxDevIndex]; }

	const VkSurfaceKHR& GetSurface() const { return m_surface; }
	VkSemaphore CreateSem() const;
	VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr) const;
	void AllocateDeviceMemory(uint64 size, uint32 memTypeIndex, VkDeviceMemory* mem);
	void AllocateImageMemory(VulkanDeviceAllocation& allocInfo, VkImage& image, VkMemoryPropertyFlags props);

	int GetQueueFamily() const { return m_gfxQueueFamily; }
	int GetTransferQueueFamily() const { return m_transferQueueFamily; }

	VkInstance& GetInstance() { return m_inst; }

	VkDevice& GetDevice() { return m_device; }

	uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;

private:
	void CreateInstance();
	//void CreateSurface();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();

	// Объекты Vulkan
	VkInstance m_inst;
	VkDevice m_device;
	VkSurfaceKHR m_surface;
	VulkanPhysicalDevices m_physDevices;

	// Внутрение детали
	std::string m_appName;
	int m_gfxDevIndex;
	int m_gfxQueueFamily;
	int m_transferQueueFamily;
	int m_presentQueueFamily;
};
