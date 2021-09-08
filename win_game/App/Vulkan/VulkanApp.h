#pragma once
#include <thread>
#include <vulkan/vulkan_core.h>
#include "Types.h"
#include "VulkanCore.h"
#include "App/App.h"
#include "App/Game.h"
#include "Types/Stack.h"
#include "UI/UINode.h"


class VulkanApp : public App
{
public:
	VulkanApp(const char* appName) :m_appName(appName), m_core(appName), vertexBuffer_(&m_core), indexBuffer_(&m_core),
		textures_{
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this),
		VulkanTextureBuffer(&m_core, this) } {}

	virtual void Init(WindowControl* winctl, Game* game);
	virtual void Start();
	virtual void Stop();
	void TransitionImageLayout(VulkanTexture& texture, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VulkanTexture& texture, VkBuffer buffer, int32 offsetX, int32 offsetY, uint32 width, uint32 height);
private:
	void CreateSwapChain();
	void CreateCommandBuffer();
	void SetScissor(VkCommandBuffer buffer, uint32 x, uint32 y, uint32 width, uint32 height);
	void ResetScissor(VkCommandBuffer buffer);
	void RecordCommandBufferForNode(VkCommandBuffer buffer, UINode* node);
	void RecordCommandBuffers(bool show = true);
	void RenderScene();
	void CreateRenderPass();
	void CreateFramebuffer();
	void CreateShaders();
	void CreateDescriptorSets();
	void CreatePipeline();
	void CreateSemaphores();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateUniformBuffer();
	void CreateDefaultTextures();
	VkSamplerCreateInfo CreateSamplerInfo(VkFilter minMagFilter, VkSamplerAddressMode addressMode, VkSamplerMipmapMode mipMapMode, float maxAniso);
	InstantCommandBuffer CreateInstantCommandBuffer(VulkanCommandPoolType type);
	void FlushInstantCommandBuffer(InstantCommandBuffer& buff);

	void Loop();
	void Update(float delta);

	std::string m_appName;
	WindowControl* m_pWindowControl;
	VulkanCore m_core;
	std::vector<VkImage> m_images;
	VkSwapchainKHR m_swapChainKHR;
	VkQueue m_queue;
	VkQueue m_transferQueue;
	std::vector<VkCommandBuffer> m_cmdBufs;
	VkCommandPool m_cmdBufPool;
	VkCommandPool m_cmdTransPool;
	std::vector<VkImageView> m_views;
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_fbs;
	VkShaderModule m_vsModule;
	VkShaderModule m_fsModule;
	VkPipeline m_pipeline;
	VkSemaphore m_renderCompleteSem;
	VkSemaphore m_presentCompleteSem;
	VkPipelineLayout m_pipelineLayout;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;
	VkBuffer m_uniformBuffer;
	VulkanUIVertexBuffer vertexBuffer_;
	VulkanIndexBuffer indexBuffer_;
	VkSampler sampler_;

	int64 lastFrameTick;
	int64 frameCount_;
	bool running_;
	std::thread reanderThread_;
	Game* game_;
	Stack<VkRect2D> scissors_;
	VulkanTextureBuffer textures_[8];
	VkDescriptorImageInfo textureDescriptors_[8];
	const uint32 TexturesCount = 8;
};
