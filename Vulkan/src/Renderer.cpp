
#include "Renderer.h"
#include "Instance.h"
#include "Logging.h"
#include "Device.h"
#include "Pipeline.h"
#include <glfw3.h>
#include "FrameBuffer.h"
#include "Commands.h"
#include "Sync.h"
#include "Descriptors.h"
#include "Descriptors.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define BUILD 0
std::string exe = "../../../Vulkan/";
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;
static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 2;
ImGui_ImplVulkanH_Window* g_wd;
static float g_tValue = 0; 
static float vel = 0; 
static float acceleration = 2;
static  glm::vec3 pos = glm::vec3(0,0,0);

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

Renderer::Renderer(int width, int height, GLFWwindow* window, bool debug):
	m_width{width},
	m_height{ height },
	m_window{window},
	m_debug{true}
{
	CreateInstance();
	CreateDevice();

	MakeDescriptorSetLayouts();

	MakePipeline();
	FinalizeSetup();
	MakeAssets();

	ImguiInit();

}

 void Renderer::SetupVulkanWindow(ImGui_ImplVulkanH_Window* g_wd, VkSurfaceKHR surface, int width, int height)
{
	g_wd->Surface = surface;

	// Check for WSI support
	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_queueIndicies.graphicsFamily.value(), g_wd->Surface, &res);
	if (res != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	// Select Surface Format
	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	g_wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_physicalDevice, g_wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

	// Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	g_wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(m_physicalDevice, g_wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	IM_ASSERT(g_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(m_instance, m_physicalDevice, m_device, g_wd, m_queueIndicies.graphicsFamily.value(), NULL, width, height, g_MinImageCount);
}
void Renderer::ImguiInit()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	g_wd = &g_MainWindowData;
	//wd->Swapchain = m_swapchain;
	g_wd->Width = w;
	g_wd->Height = h;
	g_wd->Surface = m_surface;
	g_wd->PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	g_wd->SurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	g_wd->Swapchain = m_swapchain;



	 // Create the Render Pass
	{
		VkAttachmentDescription attachment = {};
		attachment.format = g_wd->SurfaceFormat.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		auto err = vkCreateRenderPass(m_device, &info, nullptr, &g_wd->RenderPass);
		check_vk_result(err);

		// We do not create a pipeline by default as this is also used by examples' main.cpp,
		// but secondary viewport in multi-viewport mode may want to create one with:
		//ImGui_ImplVulkan_CreatePipeline(device, allocator, VK_NULL_HANDLE, wd->RenderPass, VK_SAMPLE_COUNT_1_BIT, &wd->Pipeline, bd->Subpass);
	}
	
	g_wd->ImageCount = 3;
	g_wd->Frames = new ImGui_ImplVulkanH_Frame[3];
	
	{
		VkImageView attachment[1];
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = g_wd->RenderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachment;
		info.width = g_wd->Width;
		info.height = g_wd->Height;
		info.layers = 1;
		for (uint32_t i = 0; i < g_wd->ImageCount; i++)
		{
			g_wd->Frames[i].Backbuffer = m_swapchainFrames[i].image;
			g_wd->Frames[i].BackbufferView = (VkImageView)m_swapchainFrames[i].imageView;
			g_wd->Frames[i].CommandBuffer = m_swapchainFrames[i].commandBuffer;
			ImGui_ImplVulkanH_Frame* fd = &g_wd->Frames[i];
			attachment[0] = fd->BackbufferView;
			auto err = vkCreateFramebuffer(m_device, &info, nullptr, &fd->Framebuffer);
			check_vk_result(err);
		}
	}

	


	//SetupVulkanWindow(wd, m_surface, w, h);
	ImGui::CreateContext();


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();


	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VkResult err = vkCreateDescriptorPool(m_device, &pool_info, NULL, &g_DescriptorPool);
		check_vk_result(err);
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_instance;
	init_info.PhysicalDevice = m_physicalDevice;
	init_info.Device = m_device;
	init_info.QueueFamily = m_queueIndicies.graphicsFamily.value();
	init_info.Queue = m_graphicsQueue; // or present family?
	//init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = g_MinImageCount;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = NULL;
	init_info.CheckVkResultFn = check_vk_result;
	auto err = ImGui_ImplVulkan_Init(&init_info, g_wd->RenderPass);
	// Load default font


	ImFont* font1 = io.Fonts->AddFontDefault();

	 // Upload Fonts
	{
		// Use any command queue
		//VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
		VkCommandPool command_pool = m_commandPool;
		VkCommandBuffer command_buffer = g_wd->Frames[frameNumber].CommandBuffer;

		auto err = vkResetCommandPool(m_device, command_pool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(err);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		err = vkEndCommandBuffer(command_buffer);
		check_vk_result(err);
		err = vkQueueSubmit(m_graphicsQueue, 1, &end_info, VK_NULL_HANDLE);
		check_vk_result(err);

		err = vkDeviceWaitIdle(m_device);
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
	
	
}

//reset and re-record command buffer usage mode
void Renderer::Render(float time,float deltaTime)
{
	m_device.waitForFences(1, &m_swapchainFrames[frameNumber].inFlight, VK_TRUE, UINT64_MAX);
	m_device.resetFences(1, &m_swapchainFrames[frameNumber].inFlight);


	//acquireNextImageKHR(vk::SwapChainKHR, timeout, semaphore_to_signal, fence)
	uint32_t imageIndex{ m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_swapchainFrames[frameNumber].imageAvailable, nullptr).value };

	vk::CommandBuffer commandBuffer = m_swapchainFrames[frameNumber].commandBuffer;

	commandBuffer.reset();
	
	PrepareFrame(imageIndex,time, deltaTime);

	RecordDrawCommands(commandBuffer, imageIndex);

	vk::SubmitInfo submitInfo = {};

	vk::Semaphore waitSemaphores[] = { m_swapchainFrames[frameNumber].imageAvailable };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Semaphore signalSemaphores[] = { m_swapchainFrames[frameNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	try {
		m_graphicsQueue.submit(submitInfo, m_swapchainFrames[frameNumber].inFlight);
	}
	catch (vk::SystemError err) {

		if (m_debug) {
			std::cout << "failed to submit draw command buffer!" << std::endl;
		}
	}


	



	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;
	vk::Result present;
	try {
		present = m_presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error) {
		present = vk::Result::eErrorOutOfDateKHR;
	}

	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR) {
		std::cout << "Recreate" << std::endl;
		RecreateSwapchain();
		return;
	}
	frameNumber = (frameNumber + 1) % maxFramesInFlight;
}

Renderer::~Renderer()
{
	m_device.waitIdle();

	

	glfwTerminate();
	DestroySwapchain();

	for (uint32_t i = 0; i < g_wd->ImageCount; i++)
	{
		vkDestroyFramebuffer(m_device, g_wd->Frames[i].Framebuffer, nullptr);
	}

	vkDestroyRenderPass(m_device, g_wd->RenderPass, nullptr);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	m_device.destroyPipeline(m_pipeline);
	m_device.destroyPipelineLayout(m_pipelineLayout);
	m_device.destroyRenderPass(m_renderpass);
	m_device.destroyCommandPool(m_commandPool);
	m_device.destroyDescriptorSetLayout(frameSetLayout);
	m_device.destroyDescriptorPool(frameDescriptorPool);

	m_device.destroyDescriptorSetLayout(meshSetLayout);
	m_device.destroyDescriptorPool(meshDescriptorPool);
	vkDestroyDescriptorPool(m_device, g_DescriptorPool, NULL);
	delete triangleMesh;
	delete textureNoise;
	delete textureFlameColor;
	delete textureFlame;

	
	m_device.destroy();
	m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
	m_instance.destroySurfaceKHR(m_surface);
	m_instance.destroy();
			


}



void Renderer::CreateInstance()
{
	m_instance = vkInit::CreateInstance(m_debug, "Vulkan");
	m_dldi = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
	if (m_debug)
	{
		m_debugMessenger = vkInit::make_debug_messenger(m_instance, m_dldi);	
	}


	VkSurfaceKHR c_style_surface;
	if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &c_style_surface) != VK_SUCCESS) {
		if (m_debug) {
			std::cout << "Failed to abstract glfw surface for Vulkan\n";
		}
	}
	else if (m_debug) {
		std::cout << "Successfully abstracted glfw surface for Vulkan\n";
	}
	//copy constructor converts to hpp convention
	m_surface = c_style_surface;
}
void Renderer::CreateDevice()
{
	m_physicalDevice = vkInit::ChoosePhysicalDevice(m_instance, m_debug);
	m_device  = vkInit::CreateLogicalDevice(m_physicalDevice,m_surface, m_debug);
	std::pair<std::array<vk::Queue, 2>, vkUtil::QueueFamilyIndices> queues = vkInit::GetQueue
	(m_physicalDevice, m_device, m_surface, m_debug);
	m_queueIndicies = queues.second;
	m_graphicsQueue = queues.first[0];
	m_presentQueue = queues.first[1];
	MakeSwapchain();

}

void Renderer::MakeDescriptorSetLayouts()
{
		/*
			There is just one binding, it's at binding 0,
			is a single uniform buffer, to be bound to the vertex shader stage.
		*/
		vkInit::descriptorSetLayoutData bindings;



		bindings.count = 1;
		bindings.indices.push_back(0);
		bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
		bindings.counts.push_back(1);
		bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

		frameSetLayout = vkInit::make_descriptor_set_layout(m_device, bindings);


		bindings.count = 3;
		bindings.indices[0] = 0;
		bindings.indices.push_back(1);
		bindings.indices.push_back(2);
		bindings.types[0] = (vk::DescriptorType::eCombinedImageSampler);
		bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
		bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
		bindings.counts[0] = (1);
		bindings.counts.push_back(1);
		bindings.counts.push_back(1);
		bindings.stages[0] = (vk::ShaderStageFlagBits::eFragment);
		bindings.stages.push_back(vk::ShaderStageFlagBits::eFragment);
		bindings.stages.push_back(vk::ShaderStageFlagBits::eFragment);

		meshSetLayout = vkInit::make_descriptor_set_layout(m_device, bindings);

}



void Renderer::MakePipeline()
{
	vkInit::GraphicsPipelineInBundle specification = {};
	specification.device = m_device;
	
#if BUILD
	specification.vertexFilepath = exe+"shaders/vertex.spv";
	specification.fragmentFilepath = exe+"shaders/fragment.spv";
#else
	specification.vertexFilepath = "shaders/vertex.spv";
	specification.fragmentFilepath = "shaders/fragment.spv";
#endif // BUILD

	specification.swapchainExtent = m_swapchainExtent;
	specification.swapchainImageFormat = m_swapchainFormat;
	specification.descriptorSetLayout = {frameSetLayout,meshSetLayout};

	vkInit::GraphicsPipelineOutBundle output = vkInit::create_graphics_pipeline(
		specification, m_debug
	);

	m_pipelineLayout = output.layout;
	m_renderpass = output.renderpass;
	m_pipeline = output.pipeline;
}

void Renderer::FinalizeSetup()
{
	MakeFramebuffers();


	m_commandPool = vkInit::make_command_pool(m_device, m_physicalDevice, m_surface, m_debug);
	
	vkInit::commandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapchainFrames };
	m_mainCommandBuffer = vkInit::make_command_buffer(commandBufferInput, m_debug);
	vkInit::make_frame_command_buffers(commandBufferInput, m_debug);
	
	MakeFrameResources();
}

void Renderer::MakeSwapchain()
{
	auto bundle = vkInit::CreateSwapchain(m_device, m_physicalDevice, m_surface, m_width, m_height, m_debug);
	m_swapchain = bundle.swapchain;
	m_swapchainFrames = bundle.frames;
	m_swapchainFormat = bundle.format;
	m_swapchainExtent = bundle.extent;
	maxFramesInFlight = m_swapchainFrames.size();
	frameNumber = 0;
}


void Renderer::RecreateSwapchain()
{
	m_width = 0;
	m_height = 0;
	while (m_width == 0 || m_height == 0) {
		glfwGetFramebufferSize(m_window, &m_width, &m_height);
		glfwWaitEvents();
	}


	m_device.waitIdle();

	DestroySwapchain();
	MakeSwapchain();
	MakeFramebuffers();
	MakeFrameResources();

	vkInit::commandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapchainFrames };
	vkInit::make_frame_command_buffers(commandBufferInput, m_debug);

}

void Renderer::DestroySwapchain()
{
	for (vkUtil::SwapChainFrame frame : m_swapchainFrames) {
		m_device.destroyImageView(frame.imageView);
		m_device.destroyFramebuffer(frame.framebuffer);
		m_device.destroySemaphore(frame.imageAvailable);
		m_device.destroySemaphore(frame.renderFinished);
		m_device.destroyFence(frame.inFlight);

		m_device.unmapMemory(frame.cameraDataBuffer.bufferMemory);
		m_device.freeMemory(frame.cameraDataBuffer.bufferMemory);
		m_device.destroyBuffer(frame.cameraDataBuffer.buffer);

	}

	m_device.destroySwapchainKHR(m_swapchain);

}

void Renderer::MakeFramebuffers()
{
	vkInit::framebufferInput frameBufferInput;
	frameBufferInput.device = m_device;
	frameBufferInput.renderpass = m_renderpass;
	frameBufferInput.swapchainExtent = m_swapchainExtent;
	vkInit::makeFramebuffers(frameBufferInput, m_swapchainFrames, m_debug);
}

void Renderer::MakeFrameResources()
{

	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameDescriptorPool = vkInit::make_descriptor_pool(m_device, static_cast<uint32_t>(m_swapchainFrames.size()), bindings);

	for (vkUtil::SwapChainFrame& frame : m_swapchainFrames) 
	{
		frame.imageAvailable = vkInit::make_semaphore(m_device, m_debug);
		frame.renderFinished = vkInit::make_semaphore(m_device, m_debug);
		frame.inFlight = vkInit::make_fence(m_device, m_debug);

		frame.make_ubo_resources(m_device, m_physicalDevice);

	    frame.descriptorSet = vkInit::allocate_descriptor_set(m_device, frameDescriptorPool, frameSetLayout);

	}
}

void Renderer::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo beginInfo = {};

	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		if (m_debug) {
			std::cout << "Failed to begin recording command buffer!" << std::endl;
		}
	}

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = m_renderpass;
	renderPassInfo.framebuffer = m_swapchainFrames[imageIndex].framebuffer;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = m_swapchainExtent;

	vk::ClearValue clearColor = { std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);


	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_swapchainFrames[imageIndex].descriptorSet, nullptr);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
	
	PrepareScene(commandBuffer);

	textureNoise->use(commandBuffer,m_pipelineLayout);
	textureFlame->use(commandBuffer,m_pipelineLayout);
	textureFlameColor->use(commandBuffer,m_pipelineLayout);

	//commandBuffer.draw(6, 1, 0, 0);
	commandBuffer.drawIndexed(6,1,0,0,0);

	commandBuffer.endRenderPass();

	vk::RenderPassBeginInfo renderPassInfoImGui = {};
	renderPassInfoImGui.renderPass = g_wd->RenderPass;
	renderPassInfoImGui.framebuffer = g_wd->Frames[imageIndex].Framebuffer;
	renderPassInfoImGui.renderArea.offset.x = 0;
	renderPassInfoImGui.renderArea.offset.y = 0;
	renderPassInfoImGui.renderArea.extent = m_swapchainExtent;

	renderPassInfoImGui.clearValueCount = 1;
	clearColor = { std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} };
	renderPassInfoImGui.pClearValues = &clearColor;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hello");
	ImGui::SliderFloat("t_value", &g_tValue,0,0.98);
	ImGui::End();
	ImGui::Render();
	ImDrawData* main_draw_data = ImGui::GetDrawData();

	commandBuffer.beginRenderPass(renderPassInfoImGui, vk::SubpassContents::eInline);
	ImGui_ImplVulkanH_Frame fd = g_wd->Frames[imageIndex];
	ImGui_ImplVulkan_RenderDrawData(main_draw_data, fd.CommandBuffer);
	commandBuffer.endRenderPass();

	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {

		if (m_debug) {
			std::cout << "failed to record command buffer!" << std::endl;
		}
	}
}

void Renderer::MakeAssets()
{
	triangleMesh = new Mesh(m_device, m_physicalDevice);
	//Materials

	

	//Make a descriptor pool to allocate sets.
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 3;
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshDescriptorPool = vkInit::make_descriptor_pool(m_device, 1, bindings);
	

    vkImage::TextureInputChunk textureInfoNoise;
    vkImage::TextureInputChunk textureInfoflame;
    vkImage::TextureInputChunk textureInfoflameColor;

	descriptorSetTextures = vkInit::allocate_descriptor_set(m_device, meshDescriptorPool, meshSetLayout);

#if BUILD

	auto path =  exe + "res/noise.jpg" ;
	std::cout << path.c_str() << '\n';
	textureInfoNoise.filename = path.c_str();
#else
	textureInfoNoise.filename = "res/noise.jpg";
#endif

	textureInfoNoise.commandBuffer = m_mainCommandBuffer;
	textureInfoNoise.queue = m_graphicsQueue;
	textureInfoNoise.logicalDevice = m_device;
	textureInfoNoise.physicalDevice = m_physicalDevice;
	textureInfoNoise.descriptorSet = descriptorSetTextures;


	
	textureInfoNoise.layout = meshSetLayout;
	textureInfoNoise.descriptorPool = meshDescriptorPool;
	textureInfoNoise.binding=0;
	

#if BUILD

	auto path2 = exe + "res/flame.png";

	std::cout << path2.c_str() << '\n';
	textureInfoflame.filename = path2.c_str();
#else
	textureInfoflame.filename = "res/flame.png";
#endif


	
	textureInfoflame.commandBuffer = m_mainCommandBuffer;
	textureInfoflame.queue = m_graphicsQueue;
	textureInfoflame.logicalDevice = m_device;
	textureInfoflame.physicalDevice = m_physicalDevice;



	textureInfoflame.layout = meshSetLayout;
	textureInfoflame.descriptorPool = meshDescriptorPool;
	textureInfoflame.binding=1;
	textureInfoflame.descriptorSet = descriptorSetTextures;


#if BUILD

	auto path3 = exe + "res/flameColor.png";

	std::cout << path3.c_str() << '\n';
	textureInfoflameColor.filename = path3.c_str();
#else
	textureInfoflameColor.filename = "res/flameColor.png";
#endif

	
	textureInfoflameColor.commandBuffer = m_mainCommandBuffer;
	textureInfoflameColor.queue = m_graphicsQueue;
	textureInfoflameColor.logicalDevice = m_device;
	textureInfoflameColor.physicalDevice = m_physicalDevice;



	textureInfoflameColor.layout = meshSetLayout;
	textureInfoflameColor.descriptorPool = meshDescriptorPool;
	textureInfoflameColor.binding=2;
	textureInfoflameColor.descriptorSet = descriptorSetTextures;


	textureNoise = new vkImage::Texture{ textureInfoNoise };
	textureFlame = new vkImage::Texture{ textureInfoflame };
    textureFlameColor = new vkImage::Texture{ textureInfoflameColor};
	

}



void Renderer::PrepareScene(vk::CommandBuffer commandBuffer)
{
	vk::Buffer vertexBuffers[] = { triangleMesh->vertexBuffer.buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(triangleMesh->indexBuffer.buffer,0,vk::IndexType::eUint32);
}

glm::mat4 PreparePerspectiveProjectionMatrix(float aspect_ratio,
	float field_of_view,
	float near_plane,
	float far_plane)
{

	const float tanHalfFovy = glm::tan(field_of_view / 2.f);

	auto projectionMatrix = glm::mat4{ 0.0f };
	projectionMatrix[0][0] = 1.f / (aspect_ratio * tanHalfFovy);
	projectionMatrix[1][1] = 1.f / (tanHalfFovy);
	projectionMatrix[2][2] = far_plane / (far_plane - near_plane);
	projectionMatrix[2][3] = 1.f;
	projectionMatrix[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);

	return projectionMatrix;

	float f =  glm::tan(glm::radians(0.5f * field_of_view));
	auto inverse_aspec = 1.f / aspect_ratio;
	glm::mat4 perspective_projection_matrix = {
	  inverse_aspec / f,
	  0.0f,
	  0.0f,
	  0.0f,

	  0.0f,
	  1/f,
	  0.0f,
	  0.0f,

	  0.0f,
	  0.0f,
	  far_plane / (far_plane-near_plane  ),
	  1.0f,

	  0.0f,
	  0.0f,
	  (-1*near_plane) * (far_plane-near_plane),
	  0.0f
	};
	return perspective_projection_matrix;
}

#include<gtc/matrix_access.hpp>
void Renderer::PrepareFrame(uint32_t imageIndex,float time,float deltaTime)
{
	glm::vec3 posCamera = { .0f, 0.0f,-2.0f };
	glm::vec3 target = { 0.0f, 0.0f, 0.0f };
	glm::vec3 up = { 0.0f, 1.0f, .0f };
	glm::mat4 view = glm::lookAt(posCamera, target,  up);




	float aspect = static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height), 0.1f, 100.0f);
	
	
	
	projection[1][1] *= -1;

	
	 // view matrix  is inverse(transpose) of camera transform matrix that moves objects to world zero coordinates
	 // direction of z is specified by handiness of world system (resulr of cross product)
     glm::mat4 cameraWorld{ glm::vec4{1,0,0,0},glm::vec4{glm::vec4{0,-1,0,0}},glm::vec4{0,0,-1,0},glm::vec4{0,0,2,1} };
     view = glm::inverse(cameraWorld);
	
	glm::mat4 projCustom = PreparePerspectiveProjectionMatrix(aspect, 45.0, 0.001f, 100.0f);
	float left = -20;
	float bottom = -20;
	float top = 20;
	float right = 20;
	auto ortho = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.1f, 1000.0f);




	 glm::vec4 test= { -0.5f,  0.5f,0,1 };

	glm::mat4 world = glm::mat4
	(   1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 2,
		0, 0, 0, 1
	);

	auto translate = glm::translate(glm::mat4(1.f), glm::vec3{ 0,0,12 });

	auto worldSpaceCoords = translate * test;

	
	auto input =glm::vec2( glfwGetKey(m_window,GLFW_KEY_A), glfwGetKey(m_window, GLFW_KEY_D));
	input.y *= -1;
	auto inputHoriz = input.x + input.y;
	vel += inputHoriz * acceleration * deltaTime;
	// add resist
	vel -= vel*0.01/6;
	pos.x= vel*deltaTime + pos.x;
	


	std::cout << pos.x << "\n";
	m_swapchainFrames[imageIndex].cameraData.view = view;
	m_swapchainFrames[imageIndex].cameraData.projection = projection;
	//translate *= glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_swapchainFrames[imageIndex].cameraData.viewProjection = projCustom * view;

	m_swapchainFrames[imageIndex].cameraData.world = glm::translate(glm::mat4(1.f), pos);
	
	m_swapchainFrames[imageIndex].cameraData.t = g_tValue;
	m_swapchainFrames[imageIndex].cameraData.time = time;
	m_swapchainFrames[imageIndex].cameraData.sign= -vel;
	//m_swapchainFrames[imageIndex].cameraData.viewProjection = projection;
	//m_swapchainFrames[imageIndex].cameraData.viewProjection = ortho;
	
	//std::cout << m_swapchainFrames[imageIndex].cameraData.time << "\n";
	
	memcpy(m_swapchainFrames[imageIndex].cameraDataWriteLocation, &(m_swapchainFrames[imageIndex].cameraData), sizeof(vkUtil::UBO));

	m_swapchainFrames[imageIndex].write_descriptor_set(m_device);
}




