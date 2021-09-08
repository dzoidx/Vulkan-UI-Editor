#include "VulkanApp.h"

#include <array>
#include <fstream>

#include "App/Game.h"
#include "Assets/AssetManager.h"
#include "Scene/Timer.h"
#include "UI/UIImage.h"

struct testUniforms
{
	matrix4x4 view;
};

std::array<UIInputLayout, 4> defaultInput;

uint32 defaultIndexes[] =
{
	2, 1, 0,
	0, 3, 2
};

struct RootNodeDesc
{
	VulkanUIVertexBuffer vertexBuffer;
	VulkanIndexBuffer indexBuffer;

	RootNodeDesc(VulkanCore* core) : vertexBuffer(core), indexBuffer(core) {}
};

void VulkanApp::Init(WindowControl* winctl, Game* game)
{
	game_ = game;
	m_pWindowControl = winctl;
	m_core.Init(m_pWindowControl);

	real32 w = (real32)m_pWindowControl->GetWidth();
	real32 h = (real32)m_pWindowControl->GetHeight();
	defaultInput[0].pos = vector4(w / 2, h / 2, 0, 1);
	defaultInput[0].color = Color(1);
	defaultInput[0].uv = vector2(1, 1);
	defaultInput[1].pos = vector4(10, h / 2, 0, 1);
	defaultInput[1].color = Color(1);
	defaultInput[1].uv = vector2(0, 1);
	defaultInput[2].pos = vector4(10, 10, 0, 1);
	defaultInput[2].color = Color(1);
	defaultInput[2].uv = vector2(0, 0);
	defaultInput[3].pos = vector4(w / 2, 10, 0, 1);
	defaultInput[3].color = Color(1);
	defaultInput[3].uv = vector2(1, 0);

	vkGetDeviceQueue(m_core.GetDevice(), m_core.GetQueueFamily(), 0, &m_queue);
	vkGetDeviceQueue(m_core.GetDevice(), m_core.GetTransferQueueFamily(), 0, &m_transferQueue);

	CreateSemaphores();
	CreateSwapChain();
	CreateCommandBuffer();
	CreateDefaultTextures();
	CreateRenderPass();
	CreateFramebuffer();
	CreateShaders();
	CreateUniformBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateDescriptorSets();
	CreatePipeline();
	RecordCommandBuffers();

}

void VulkanApp::CreateSwapChain()
{
	const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_core.GetSurfaceCaps();

	assert(SurfaceCaps.currentExtent.width != -1);

	uint32_t NumImages = 2;

	assert(NumImages >= SurfaceCaps.minImageCount);
	assert(NumImages <= SurfaceCaps.maxImageCount);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};

	SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainCreateInfo.surface = m_core.GetSurface();
	SwapChainCreateInfo.minImageCount = NumImages;
	SwapChainCreateInfo.imageFormat = m_core.GetSurfaceFormat().format;
	SwapChainCreateInfo.imageColorSpace = m_core.GetSurfaceFormat().colorSpace;
	SwapChainCreateInfo.imageExtent = SurfaceCaps.currentExtent;
	SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	SwapChainCreateInfo.imageArrayLayers = 1;
	SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	SwapChainCreateInfo.clipped = true;
	SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	VkResult res = vkCreateSwapchainKHR(m_core.GetDevice(), &SwapChainCreateInfo, nullptr, &m_swapChainKHR);
	assert(res == 0);

	uint32_t NumSwapChainImages = 0;
	res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, nullptr);

	m_images.resize(NumSwapChainImages);
	m_cmdBufs.resize(NumSwapChainImages);
	m_views.resize(NumSwapChainImages);

	res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, &(m_images[0]));
	assert(res == 0);
}

void VulkanApp::CreateCommandBuffer()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.queueFamilyIndex = m_core.GetQueueFamily();

	VkResult res = vkCreateCommandPool(m_core.GetDevice(), &cmdPoolCreateInfo, nullptr, &m_cmdBufPool);
	assert(res == 0);

	VkCommandPoolCreateInfo cmdTransPoolCreateInfo = {};
	cmdTransPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdTransPoolCreateInfo.queueFamilyIndex = m_core.GetTransferQueueFamily();

	res = vkCreateCommandPool(m_core.GetDevice(), &cmdTransPoolCreateInfo, nullptr, &m_cmdTransPool);
	assert(res == 0);

	VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
	cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocInfo.commandPool = m_cmdBufPool;
	cmdBufAllocInfo.commandBufferCount = (uint32_t)m_images.size();
	cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	res = vkAllocateCommandBuffers(m_core.GetDevice(), &cmdBufAllocInfo, &m_cmdBufs[0]);
	assert(res == 0);
}

void VulkanApp::RecordCommandBufferForNode(VkCommandBuffer buffer, UINode* node)
{
	VkDeviceSize offsets[1] = { 0 };

	bool resetScissor = false;
	if (node->Type == UINodeType::Image)
	{
		UIImage* img = (UIImage*)node;
		if (img->Mask)
		{
			resetScissor = img->Mask;
			SetScissor(buffer, (uint32)img->AABB.x, (uint32)img->AABB.y, (uint32)img->AABB.w, (uint32)img->AABB.h);
		}
		RootNodeDesc* desc = (RootNodeDesc*)img->RenderData;
		if (desc == nullptr)
		{
			desc = new RootNodeDesc(&m_core);
			desc->vertexBuffer.Init(img->Geometry.GetData(), (uint32)img->Geometry.GetLen());
			desc->indexBuffer.Init(img->GeometryIndexes.GetData(), (uint32)img->GeometryIndexes.GetLen());
			img->RenderData = desc;
		}
		vkCmdPushConstants(
			buffer,
			m_pipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(int),
			(void*)&img->TextureSlot);
		vkCmdBindVertexBuffers(buffer, 0, 1, desc->vertexBuffer.GetBuffer(), offsets);
		vkCmdBindIndexBuffer(buffer, desc->indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(buffer, (uint32)img->GeometryIndexes.GetLen(), 1, 0, 0, 0);
	}
	for (uint32 i = 0; i < node->Children.GetLen(); ++i)
	{
		RecordCommandBufferForNode(buffer, node->Children[i]);
	}
	if (resetScissor)
	{
		ResetScissor(buffer);
	}
}

void VulkanApp::SetScissor(VkCommandBuffer buffer, uint32 x, uint32 y, uint32 width, uint32 height)
{
	VkRect2D scissor{};
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = width;
	scissor.extent.height = height;
	vkCmdSetScissor(buffer, 0, 1, &scissor);
	scissors_.Push(scissor);
}

void VulkanApp::ResetScissor(VkCommandBuffer buffer)
{
	scissors_.Pop();
	VkRect2D scissor = scissors_.Peek();
	vkCmdSetScissor(buffer, 0, 1, &scissor);
}

void VulkanApp::RecordCommandBuffers(bool show)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkClearColorValue clearColor = { 164.0f / 256.0f, 30.0f / 256.0f, 34.0f / 256.0f, 0.0f };
	VkClearValue clearValue = {};
	clearValue.color = clearColor;

	VkImageSubresourceRange imageRange = {};
	imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageRange.levelCount = 1;
	imageRange.layerCount = 1;

	uint32_t w = m_pWindowControl->GetWidth();
	uint32_t h = m_pWindowControl->GetHeight();
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent.width = w;
	renderPassInfo.renderArea.extent.height = h;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	for (uint32_t i = 0; i < m_cmdBufs.size(); i++) {
		VkResult res = vkBeginCommandBuffer(m_cmdBufs[i], &beginInfo);
		assert(res == 0);
		renderPassInfo.framebuffer = m_fbs[i];

		vkCmdBeginRenderPass(m_cmdBufs[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		SetScissor(m_cmdBufs[i], 0, 0, m_pWindowControl->GetWidth(), m_pWindowControl->GetHeight());

		vkCmdBindPipeline(m_cmdBufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindDescriptorSets(m_cmdBufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

		UINode* node = nullptr;
		if (game_ != nullptr)
			node = game_->GetSceneRoot();

		if (node != nullptr)
			RecordCommandBufferForNode(m_cmdBufs[i], node);

		if (show)
		{
			vkCmdBindVertexBuffers(m_cmdBufs[i], 0, 1, vertexBuffer_.GetBuffer(), offsets);
			vkCmdBindIndexBuffer(m_cmdBufs[i], indexBuffer_.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(m_cmdBufs[i], 6, 1, 0, 0, 0);
		}
		//vkCmdDraw(m_cmdBufs[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(m_cmdBufs[i]);

		res = vkEndCommandBuffer(m_cmdBufs[i]);
		assert(res == 0);
	}
}

void VulkanApp::RenderScene()
{
	uint32_t ImageIndex = 0;

	VkResult res = vkAcquireNextImageKHR(m_core.GetDevice(), m_swapChainKHR, UINT64_MAX, m_presentCompleteSem, NULL, &ImageIndex);
	assert(res == 0);

	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_cmdBufs[ImageIndex];
	submitInfo.pWaitSemaphores = &m_presentCompleteSem;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = &waitFlags;
	submitInfo.pSignalSemaphores = &m_renderCompleteSem;
	submitInfo.signalSemaphoreCount = 1;

	res = vkQueueSubmit(m_queue, 1, &submitInfo, NULL);
	assert(res == 0);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChainKHR;
	presentInfo.pImageIndices = &ImageIndex;
	presentInfo.pWaitSemaphores = &m_renderCompleteSem;
	presentInfo.waitSemaphoreCount = 1;

	res = vkQueuePresentKHR(m_queue, &presentInfo);
	assert(res == 0);
}

void VulkanApp::CreateRenderPass()
{
	VkAttachmentReference attachRef = {};
	attachRef.attachment = 0;
	attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &attachRef;

	VkAttachmentDescription attachDesc = {};
	attachDesc.format = m_core.GetSurfaceFormat().format;
	attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachDesc;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDesc;

	VkResult res = vkCreateRenderPass(m_core.GetDevice(), &renderPassCreateInfo, NULL, &m_renderPass);
	assert(res == 0);
}

void VulkanApp::CreateFramebuffer()
{
	m_fbs.resize(m_images.size());

	for (uint32_t i = 0; i < m_images.size(); i++) {
		VkImageViewCreateInfo ViewCreateInfo = {};
		ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewCreateInfo.image = m_images[i];
		ViewCreateInfo.format = m_core.GetSurfaceFormat().format;
		ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ViewCreateInfo.subresourceRange.levelCount = 1;
		ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ViewCreateInfo.subresourceRange.layerCount = 1;

		VkResult res = vkCreateImageView(m_core.GetDevice(), &ViewCreateInfo, NULL, &m_views[i]);
		assert(res == 0);

		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.renderPass = m_renderPass;
		fbCreateInfo.attachmentCount = 1;
		fbCreateInfo.pAttachments = &m_views[i];
		fbCreateInfo.width = m_pWindowControl->GetWidth();
		fbCreateInfo.height = m_pWindowControl->GetHeight();
		fbCreateInfo.layers = 1;

		res = vkCreateFramebuffer(m_core.GetDevice(), &fbCreateInfo, NULL, &m_fbs[i]);
		assert(res == 0);
	}
}

VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName)
{
	std::ifstream t(pFileName, std::ios::binary);
	t.seekg(0, std::ios::end);
	int size = (int)t.tellg();
	t.seekg(0, std::ios::beg);
	char* pShaderCode = new char[size];
	t.read(pShaderCode, size);
	assert(pShaderCode);

	VkShaderModuleCreateInfo shaderCreateInfo = {};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = size;
	shaderCreateInfo.pCode = (const uint32_t*)pShaderCode;

	VkShaderModule shaderModule;
	VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, NULL, &shaderModule);
	assert(res == 0);
	return shaderModule;
}

void VulkanApp::CreateShaders()
{
	//m_vsModule = VulkanCreateShaderModule(m_core.GetDevice(), "G:\\src\\exot\\Assets\\\Shaders\\GLSL\\vk_simple_triangle.vs.spv");
	m_vsModule = VulkanCreateShaderModule(m_core.GetDevice(), "E:\\src\\exot\\Assets\\Shaders\\GLSL\\vk_simple_color.vs.spv");
	assert(m_vsModule);

	//m_fsModule = VulkanCreateShaderModule(m_core.GetDevice(), "G:\\src\\exot\\Assets\\\Shaders\\GLSL\\vk_simple_triangle.fs.spv");
	m_fsModule = VulkanCreateShaderModule(m_core.GetDevice(), "E:\\src\\exot\\Assets\\Shaders\\GLSL\\vk_simple_color.fs.spv");
	assert(m_fsModule);
}

void VulkanApp::CreateDescriptorSets()
{
	std::array<VkDescriptorSetLayoutBinding, 3> setLayoutBindings{};

	setLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	setLayoutBindings[0].binding = 0;
	setLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	setLayoutBindings[0].descriptorCount = 1;

	setLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	setLayoutBindings[1].binding = 1;
	setLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBindings[1].descriptorCount = 1;

	setLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	setLayoutBindings[2].binding = 2;
	setLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBindings[2].descriptorCount = TexturesCount;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorLayoutCI.pBindings = setLayoutBindings.data();

	VkResult res = vkCreateDescriptorSetLayout(m_core.GetDevice(), &descriptorLayoutCI, nullptr, &m_descriptorSetLayout);
	assert(res == 0);

	std::array<VkDescriptorPoolSize, 3> descriptorPoolSizes{};
	descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSizes[0].descriptorCount = 1;
	descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSizes[1].descriptorCount = 1;
	descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSizes[2].descriptorCount = TexturesCount;

	VkDescriptorPoolCreateInfo descriptorPoolCI = {};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCI.pPoolSizes = descriptorPoolSizes.data();
	descriptorPoolCI.maxSets = 2 + TexturesCount;

	res = vkCreateDescriptorPool(m_core.GetDevice(), &descriptorPoolCI, nullptr, &m_descriptorPool);
	assert(res == 0);

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = m_descriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &m_descriptorSetLayout;

	res = vkAllocateDescriptorSets(m_core.GetDevice(), &allocateInfo, &m_descriptorSet);
	assert(res == 0);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = m_uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(testUniforms);

	VkSamplerCreateInfo samplerInfo = CreateSamplerInfo(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f);
	res = vkCreateSampler(m_core.GetDevice(), &samplerInfo, 0, &sampler_);
	assert(res == 0);
	VkDescriptorImageInfo samplerDescInfo{};
	samplerDescInfo.sampler = sampler_;

	std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};
	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = m_descriptorSet;
	writeDescriptorSets[0].dstBinding = 0;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[0].pBufferInfo = &bufferInfo;
	writeDescriptorSets[0].descriptorCount = 1;

	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].dstSet = m_descriptorSet;
	writeDescriptorSets[1].dstBinding = 1;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].pBufferInfo = 0;
	writeDescriptorSets[1].pImageInfo = &samplerDescInfo;

	writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[2].dstSet = m_descriptorSet;
	writeDescriptorSets[2].dstBinding = 2;
	writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeDescriptorSets[2].descriptorCount = TexturesCount;
	writeDescriptorSets[2].pBufferInfo = 0;
	writeDescriptorSets[2].pImageInfo = textureDescriptors_;

	vkUpdateDescriptorSets(m_core.GetDevice(), (uint32)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanApp::CreatePipeline()
{
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};

	shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = m_vsModule;
	shaderStageCreateInfo[0].pName = "main";
	shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = m_fsModule;
	shaderStageCreateInfo[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	std::vector<VkVertexInputAttributeDescription> attributes(3);
	// position
	attributes[0].location = 0;
	attributes[0].binding = 0;
	attributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[0].offset = 0;

	// color
	attributes[1].location = 1;
	attributes[1].binding = 0;
	attributes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[1].offset = sizeof(vector4);

	// uv
	attributes[2].location = 2;
	attributes[2].binding = 0;
	attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[2].offset = sizeof(vector4) * 2;

	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

	VkVertexInputBindingDescription vInputBindDescription{};
	vInputBindDescription.binding = 0;
	vInputBindDescription.stride = sizeof(vector4) * 2 + sizeof(vector2);
	vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vInputBindDescription;

	VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
	pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_pWindowControl->GetWidth();
	vp.height = (float)m_pWindowControl->GetHeight();
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;

	VkRect2D scissor; // ignored. see dynamicState
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = m_pWindowControl->GetWidth();
	scissor.extent.height = m_pWindowControl->GetHeight();

	VkPipelineViewportStateCreateInfo vpCreateInfo = {};
	vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpCreateInfo.viewportCount = 1;
	vpCreateInfo.pViewports = &vp;
	vpCreateInfo.scissorCount = 1;
	vpCreateInfo.pScissors = &scissor;

	VkPipelineDepthStencilStateCreateInfo dsInfo = {};
	dsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
	rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rastCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
	pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMSCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState blendAttachState = {};
	blendAttachState.colorWriteMask = 0xf;

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = &blendAttachState;

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(int);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &m_descriptorSetLayout;
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &pushConstantRange;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStates;
	dynamicState.dynamicStateCount = 1;
	dynamicState.flags = false;

	VkResult res = vkCreatePipelineLayout(m_core.GetDevice(), &layoutInfo, nullptr, &m_pipelineLayout);
	assert(res == 0);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = &shaderStageCreateInfo[0];
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
	pipelineInfo.pViewportState = &vpCreateInfo;
	pipelineInfo.pDepthStencilState = &dsInfo;
	pipelineInfo.pRasterizationState = &rastCreateInfo;
	pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
	pipelineInfo.pColorBlendState = &blendCreateInfo;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.basePipelineIndex = -1;
	res = vkCreateGraphicsPipelines(m_core.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &m_pipeline);
	assert(res == 0);
}

void VulkanApp::CreateSemaphores()
{
	m_renderCompleteSem = m_core.CreateSem();
	m_presentCompleteSem = m_core.CreateSem();
}

void VulkanApp::CreateVertexBuffer()
{
	vertexBuffer_.Init(defaultInput.data(), (uint32)defaultInput.size());
}

void VulkanApp::CreateIndexBuffer()
{
	indexBuffer_.Init(defaultIndexes, 6);
}

void VulkanApp::CreateUniformBuffer()
{
	real32 w = (real32)m_pWindowControl->GetWidth();
	real32 h = (real32)m_pWindowControl->GetHeight();
	VkDeviceMemory memory; // TODO: free memory
	testUniforms uniforms;
	// vulkan's matrises must be pushed in column major order
	uniforms.view = Transpose(orthographicLeftHandedYFlipped(0, w - 1, 0, h - 1, -1, 1));

	//vector4 v0 = vector4(w / 2, h / 2, 0, 1) * uniforms.view;
	//vector4 v1 = vector4(10, h / 2, 0, 1) * uniforms.view;
	//vector4 v2 = vector4(10, 10, 0, 1) * uniforms.view;
	//vector4 v3 = vector4(w / 2, 10, 0, 1) * uniforms.view;

	m_core.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(testUniforms),
		&m_uniformBuffer, &memory, &uniforms);
}

void VulkanApp::CreateDefaultTextures()
{
	TextureAsset t;
	t.BytesPerPixel = 4;
	t.Width = 2;
	t.Height = 2;

	byte white[16] = { 255,255,255,255, 255,255,255,255,
						255,255,255,255, 255,255,255,255 };
	byte red[16] = { 255,0,0,255, 255,0,0,255,
						255,0,0,255, 255,0,0,255 };
	byte green[16] = { 0,255,0,255, 0,255,0,255,
						0,255,0,255, 0,255,0,255 };
	byte blue[16] = { 0,0,255,255, 0,0,255,255,
						0,0,255,255, 0,0,255,255 };
	byte magenta[16] = { 240,0,130,255, 240,0,130,255,
						240,0,130,255, 240,0,130,255 };
	byte yellow[16] = { 230,220,50,255, 230,220,50,255,230,220,50,255,230,220,50,255 };
	byte orange[16] = { 240,130,40,255, 240,130,40,255, 240,130,40,255, 240,130,40,255, };
	byte purple[16] = { 160,0,200,255, 160,0,200,255,160,0,200,255,160,0,200,255 };

	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, white);
	textures_[0].Init(t);
	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, red);
	textures_[1].Init(t);
	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, green);
	textures_[2].Init(t);
	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, blue);
	textures_[3].Init(t);
	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, magenta);
	textures_[4].Init(t);
	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, yellow);
	textures_[5].Init(t);
	t = AssetManager::GetInstance()->GenerateTexture(true, 2, 2, orange);
	textures_[6].Init(t);
	{
		TextureAsset texture = AssetManager::GetInstance()->LoadTexture("Textures/test.et");
		texture.LoadSync();
		t.BytesPerPixel = texture.Alpha ? 4 : 3;
		t.Height = texture.Height;
		t.Width = texture.Width;
		t.Data = texture.Data;
		textures_[7].Init(t);
	}

	for (uint32 i = 0; i < TexturesCount; ++i)
	{
		textureDescriptors_[i] = {};
		textureDescriptors_[i].imageView = textures_[i].TextureInfo.View;
		textureDescriptors_[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}

VkSamplerCreateInfo VulkanApp::CreateSamplerInfo(VkFilter minMagFilter, VkSamplerAddressMode addressMode, VkSamplerMipmapMode mipMapMode, float maxAniso)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = minMagFilter;
	samplerInfo.minFilter = minMagFilter;

	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;

	samplerInfo.anisotropyEnable = maxAniso > 0.01f ? VK_TRUE : VK_FALSE;
	samplerInfo.maxAnisotropy = maxAniso;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	//mostly for pcf? 
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = mipMapMode;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	return samplerInfo;
}

InstantCommandBuffer VulkanApp::CreateInstantCommandBuffer(VulkanCommandPoolType type)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (type == VulkanCommandPoolType::Graphics)
		allocInfo.commandPool = m_cmdBufPool;
	else if (type == VulkanCommandPoolType::Transfer)
		allocInfo.commandPool = m_cmdTransPool;
	else
		assert(0); // no present command pool
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VkResult res = vkAllocateCommandBuffers(m_core.GetDevice(), &allocInfo, &commandBuffer);
	assert(res == 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	assert(res == 0);

	InstantCommandBuffer r;
	r.Buffer = commandBuffer;
	r.Type = type;
	return r;
}

void VulkanApp::FlushInstantCommandBuffer(InstantCommandBuffer& buff)
{
	VkResult res = vkEndCommandBuffer(buff.Buffer);
	assert(res == 0);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buff.Buffer;

	VkQueue queue = nullptr;
	VkCommandPool pool = nullptr;
	if (buff.Type == VulkanCommandPoolType::Graphics)
	{
		queue = m_queue;
		pool = m_cmdBufPool;
	}
	else if (buff.Type == VulkanCommandPoolType::Transfer)
	{
		queue = m_transferQueue;
		pool = m_cmdTransPool;
	}
	else
	{
		assert(0);
	}

	res = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(res == 0);
	res = vkQueueWaitIdle(queue);
	assert(res == 0);

	vkFreeCommandBuffers(m_core.GetDevice(), pool, 1, &buff.Buffer);
}

void VulkanApp::TransitionImageLayout(VulkanTexture& texture, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	InstantCommandBuffer buff = CreateInstantCommandBuffer(VulkanCommandPoolType::Graphics);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	//these are used to transfer queue ownership, which we aren't doing
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = texture.Image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		assert(0);
	}

	vkCmdPipelineBarrier(
		buff.Buffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);


	FlushInstantCommandBuffer(buff);
}

void VulkanApp::CopyBufferToImage(VulkanTexture& texture, VkBuffer buffer, int32 offsetX, int32 offsetY, uint32 width, uint32 height)
{
	InstantCommandBuffer buff = CreateInstantCommandBuffer(VulkanCommandPoolType::Transfer);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { offsetX, offsetY, 0 };
	region.imageExtent = { width, height, 1 };


	vkCmdCopyBufferToImage(
		buff.Buffer,
		buffer,
		texture.Image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	FlushInstantCommandBuffer(buff);
}

void VulkanApp::Update(float delta)
{
	static bool isOn = true;
	static float timePassed;
	static float totalTime;
	if (game_ != nullptr)
		game_->Update(delta);
	timePassed += delta;
	totalTime += delta;
	/*
	if (timePassed > 1.0f)
	{
		timePassed = 0.0f;
		isOn = !isOn;
		RecordCommandBuffers(isOn);
		Debug(String::Format("Show white rect={0}.").Set(0, (int32)isOn).ToUtf8());
	}*/
}

void VulkanApp::Loop()
{
	while (running_)
	{
		float deltaTime = 0.0f;
		if (frameCount_)
		{
			int64 curTick = GetCurrentMsecs();
			deltaTime = (curTick - lastFrameTick) / 1000.0f;
		}
		lastFrameTick = GetCurrentMsecs();
		++frameCount_;
		Update(deltaTime);
		if (game_ != nullptr && game_->IsDirty())
			RecordCommandBuffers(true);
		RenderScene();
	}
}

void VulkanApp::Start()
{
	if (running_)
		return;
	running_ = true;

	reanderThread_ = std::thread(&VulkanApp::Loop, this);
}

void VulkanApp::Stop()
{
	if (!running_)
		return;
	running_ = false;
	reanderThread_.join();
}