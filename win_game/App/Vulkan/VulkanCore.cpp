#include "VulkanCore.h"

#include <vulkan/vulkan_win32.h>

#include "Logging.h"
#include "VulkanApp.h"
#include "UI/UINode.h"
#include "Utils/StringBuilder.h"

void VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps)
{
	unsigned int NumExt = 0;
	VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &NumExt, nullptr);
	assert(res == 0);

	ExtProps.resize(NumExt);

	res = vkEnumerateInstanceExtensionProperties(nullptr, &NumExt, &ExtProps[0]);
	assert(res == 0);
}

void VulkanGetPhysicalDevices(const VkInstance& inst, const VkSurfaceKHR& Surface, VulkanPhysicalDevices& PhysDevices)
{
	unsigned int NumDevices = 0;

	VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, nullptr);
	assert(res == 0);

	PhysDevices.m_devices.resize(NumDevices);
	PhysDevices.m_devProps.resize(NumDevices);
	PhysDevices.m_qFamilyProps.resize(NumDevices);
	PhysDevices.m_qSupportsPresent.resize(NumDevices);
	PhysDevices.m_surfaceFormats.resize(NumDevices);
	PhysDevices.m_surfaceCaps.resize(NumDevices);
	PhysDevices.m_devFeatures.resize(NumDevices);
	PhysDevices.m_devMemProps.resize(NumDevices);

	res = vkEnumeratePhysicalDevices(inst, &NumDevices, &PhysDevices.m_devices[0]);
	assert(res == 0);

	for (unsigned int i = 0; i < NumDevices; i++)
	{
		const VkPhysicalDevice& PhysDev = PhysDevices.m_devices[i];
		vkGetPhysicalDeviceProperties(PhysDev, &PhysDevices.m_devProps[i]);
		uint32_t apiVer = PhysDevices.m_devProps[i].apiVersion;
		auto maj = VK_VERSION_MAJOR(apiVer);
		auto min = VK_VERSION_MINOR(apiVer);
		auto patch = VK_VERSION_PATCH(apiVer);

		uint32_t NumQFamily = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, NULL);

		PhysDevices.m_qFamilyProps[i].resize(NumQFamily);
		PhysDevices.m_qSupportsPresent[i].resize(NumQFamily);

		vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, &(PhysDevices.m_qFamilyProps[i][0]));

		for (uint32_t q = 0; q < NumQFamily; q++)
		{
			res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, Surface, &(PhysDevices.m_qSupportsPresent[i][q]));
			assert(res == 0);
		}

		uint32_t NumFormats = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, NULL);
		assert(NumFormats > 0);

		PhysDevices.m_surfaceFormats[i].resize(NumFormats);

		res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, &(PhysDevices.m_surfaceFormats[i][0]));
		assert(res == 0);

		res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDev, Surface, &(PhysDevices.m_surfaceCaps[i]));
		assert(res == 0);

		vkGetPhysicalDeviceFeatures(PhysDev, &PhysDevices.m_devFeatures[i]);
		vkGetPhysicalDeviceMemoryProperties(PhysDev, &PhysDevices.m_devMemProps[i]);
	}
}

bool VulkanCore::Init(WindowControl* pWindowControl)
{
	std::vector<VkExtensionProperties> ExtProps;
	VulkanEnumExtProps(ExtProps);

	CreateInstance();

	m_surface = pWindowControl->CreateSurface(m_inst);

	VulkanGetPhysicalDevices(m_inst, m_surface, m_physDevices);
	SelectPhysicalDevice();
	CreateLogicalDevice();

	return true;
}

void VulkanCore::CreateInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_appName.c_str();
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	const char* pInitExt[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif _ANDROID
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif _IOS
		VK_MVK_IOS_SURFACE_EXTENSION_NAME
#elif _LINUX
		VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif
	};

	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pApplicationInfo = &appInfo;
	instInfo.enabledExtensionCount = 2;
	instInfo.ppEnabledExtensionNames = pInitExt;

	VkResult result = vkCreateInstance(&instInfo, nullptr, &m_inst);
	assert(result == 0);
}

void VulkanCore::SelectPhysicalDevice()
{
	for (uint32_t i = 0; i < m_physDevices.m_devices.size(); i++) {

		for (uint32_t j = 0; j < m_physDevices.m_qFamilyProps[i].size(); j++) {
			VkQueueFamilyProperties& QFamilyProp = m_physDevices.m_qFamilyProps[i][j];

			Debug(String::Format("Family {0} Num queues: {1}.")
				.Set(0, j)
				.Set(1, QFamilyProp.queueCount)
				.ToUtf8());
			VkQueueFlags flags = QFamilyProp.queueFlags;
			Debug(String::Format("    GFX {0}, Compute {1}, Transfer {2}, Sparse binding {3}.")
				.Set(0, (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No")
				.Set(1, (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No")
				.Set(2, (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No")
				.Set(3, (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No")
				.ToUtf8());

			if (flags & VK_QUEUE_GRAPHICS_BIT) {
				if (!m_physDevices.m_qSupportsPresent[i][j]) {
					Debug("Present is not supported.");
					continue;
				}

				m_gfxDevIndex = i;
				m_gfxQueueFamily = j;
				Debug(String::Format("Using GFX device {0} and queue family {1}.")
					.Set(0, m_gfxDevIndex)
					.Set(1, m_gfxQueueFamily)
					.ToUtf8());
				break;
			}
		}
	}

	if (m_gfxDevIndex == -1) {
		Debug("No GFX device found!");
		assert(0);
	}

	for (uint32_t j = 0; j < m_physDevices.m_qFamilyProps[m_gfxDevIndex].size(); j++) {
		VkQueueFamilyProperties& QFamilyProp = m_physDevices.m_qFamilyProps[m_gfxDevIndex][j];

		Debug(String::Format("Family {0} Num queues: {1}.")
			.Set(0, j)
			.Set(1, QFamilyProp.queueCount)
			.ToUtf8());
		VkQueueFlags flags = QFamilyProp.queueFlags;

		if (flags & VK_QUEUE_TRANSFER_BIT) {

			m_transferQueueFamily = j;
			Debug(String::Format("Using transfer queue family {0}.")
				.Set(0, m_transferQueueFamily)
				.ToUtf8());
			break;
		}
	}
}

void VulkanCore::CreateLogicalDevice()
{
	float qPriorities = 1.0f;
	VkDeviceQueueCreateInfo qInfo = {};
	qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qInfo.queueFamilyIndex = m_gfxQueueFamily;
	qInfo.queueCount = 1;
	qInfo.pQueuePriorities = &qPriorities;

	const char* pDevExt[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo devInfo = {};
	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.enabledExtensionCount = 1;
	devInfo.ppEnabledExtensionNames = pDevExt;
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &qInfo;

	VkResult res = vkCreateDevice(GetPhysDevice(), &devInfo, NULL, &m_device);

	assert(res == 0);

	Debug("Device created.");
}

VkSemaphore VulkanCore::CreateSem() const
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore semaphore;
	VkResult res = vkCreateSemaphore(m_device, &createInfo, nullptr, &semaphore);
	assert(res == 0);
	return semaphore;
}

VkResult VulkanCore::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data) const
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = size;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult res = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, buffer);
	assert(res == 0);

	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkGetBufferMemoryRequirements(m_device, *buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	VkBool32 ok;
	memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, &ok);
	assert(ok);
	res = vkAllocateMemory(m_device, &memAlloc, nullptr, memory);
	assert(res == 0);

	if (data != nullptr)
	{
		void* mapped;
		VkResult res = vkMapMemory(m_device, *memory, 0, size, 0, &mapped);
		assert(res == 0);
		memcpy(mapped, data, size);
		if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = *memory;
			mappedRange.offset = 0;
			mappedRange.size = size;
			vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
		}
		vkUnmapMemory(m_device, *memory);
	}

	vkBindBufferMemory(m_device, *buffer, *memory, 0);

	return VK_SUCCESS;
}

void VulkanCore::AllocateDeviceMemory(uint64 size, uint32 memTypeIndex, VkDeviceMemory* mem)
{
	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = size;
	memAlloc.memoryTypeIndex = memTypeIndex;

	VkResult res = vkAllocateMemory(m_device, &memAlloc, 0, mem);
	assert(res == 0);
}

void VulkanCore::AllocateImageMemory(VulkanDeviceAllocation& allocInfo, VkImage& image, VkMemoryPropertyFlags props)
{
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(m_device, image, &memReqs);

	allocInfo.Size = memReqs.size;

	VkBool32 ok;
	uint32 type = GetMemoryType(memReqs.memoryTypeBits, props, &ok);
	assert(ok);

	AllocateDeviceMemory(memReqs.size, type, &allocInfo.Memory);
}

uint32_t VulkanCore::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
	int count = m_physDevices.m_devMemProps[m_gfxDevIndex].memoryTypeCount;
	for (int i = 0; i < count; ++i)
	{
		if ((typeBits & 1) == 1)
		{
			if ((m_physDevices.m_devMemProps[m_gfxDevIndex].memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
					*memTypeFound = true;
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
		*memTypeFound = false;


	return 0;
}

void VulkanBuffer::Init(VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkDeviceSize size, void* data)
{
	FreeResources();
	hostCoherent_ = (props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VkResult res = core_->CreateBuffer(usage, props, size, &buffer_, &memory_, data);
	assert(res == 0);
}

VulkanBuffer::~VulkanBuffer()
{
	FreeResources();
}

void VulkanBuffer::FreeResources()
{
	VkDevice device = core_->GetDevice();
	if (memory_ != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, memory_, nullptr);
		memory_ = VK_NULL_HANDLE;
	}
	if (buffer_ != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(device, buffer_, nullptr);
		buffer_ = VK_NULL_HANDLE;
	}
}

void VulkanBuffer::Update(void* data, VkDeviceSize offset, VkDeviceSize size)
{
	VkDevice device = core_->GetDevice();
	void* mapped;
	// VkMemoryMapFlags is a bitmask type for setting a mask, but is currently reserved for future use. (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html)
	VkResult res = vkMapMemory(device, memory_, offset, size, 0, &mapped);
	assert(res == 0);
	memcpy(mapped, data, size);
	if (!hostCoherent_)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory_;
		mappedRange.offset = offset;
		mappedRange.size = size;
		vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	}
	vkUnmapMemory(device, memory_);
}

void VulkanUIVertexBuffer::Init(UIInputLayout* data, uint32 count)
{
	VkDeviceSize size = sizeof(UIInputLayout) * count;
	VulkanBuffer::Init(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		size,
		data);
}

void VulkanUIVertexBuffer::Update(UIInputLayout* data, uint32 index, uint32 count)
{
	VkDeviceSize size = sizeof(UIInputLayout) * count;
	VkDeviceSize offset = sizeof(UIInputLayout) * index;
	VulkanBuffer::Update(data, offset, size);
}

void VulkanIndexBuffer::Init(uint32* data, uint32 count)
{
	VkDeviceSize size = sizeof(uint32) * count;
	VulkanBuffer::Init(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		size,
		data);
}

void VulkanIndexBuffer::Update(uint32* data, uint32 index, uint32 count)
{
	VkDeviceSize size = sizeof(uint32) * count;
	VkDeviceSize offset = sizeof(uint32) * index;
	VulkanBuffer::Update(data, offset, size);
}

void VulkanTextureBuffer::Init(TextureAsset& data)
{
	FreeResources();
	VkDeviceSize size = data.Width * data.Height * data.BytesPerPixel;
	VulkanBuffer::Init(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		size,
		data.Data);

	TextureInfo = {};
	TextureInfo.Format = VK_FORMAT_R8G8B8A8_UNORM;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = data.Width;
	imageInfo.extent.height = data.Height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = TextureInfo.Format;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult res = vkCreateImage(core_->GetDevice(), &imageInfo, nullptr, &TextureInfo.Image);
	assert(res == 0);

	core_->AllocateImageMemory(TextureInfo.AllocInfo, TextureInfo.Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	res = vkBindImageMemory(core_->GetDevice(), TextureInfo.Image, TextureInfo.AllocInfo.Memory, 0);
	assert(res == 0);

	app_->TransitionImageLayout(TextureInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	app_->CopyBufferToImage(TextureInfo, buffer_, 0, 0, data.Width, data.Height);

	app_->TransitionImageLayout(TextureInfo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = TextureInfo.Image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = TextureInfo.Format;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	res = vkCreateImageView(core_->GetDevice(), &createInfo, nullptr, &TextureInfo.View);
	assert(res == 0);
}

void VulkanTextureBuffer::FreeResources()
{
	if (TextureInfo.View != VK_NULL_HANDLE)
	{
		vkDestroyImageView(core_->GetDevice(), TextureInfo.View, 0);
		TextureInfo.View = VK_NULL_HANDLE;
	}
	if (TextureInfo.AllocInfo.Memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(core_->GetDevice(), TextureInfo.AllocInfo.Memory, 0);
		TextureInfo.AllocInfo.Memory = VK_NULL_HANDLE;
	}
	if (TextureInfo.Image != VK_NULL_HANDLE)
	{
		vkDestroyImage(core_->GetDevice(), TextureInfo.Image, 0);
		TextureInfo.Image = VK_NULL_HANDLE;
	}
	VulkanBuffer::FreeResources();
}