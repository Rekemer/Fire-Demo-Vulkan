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



Renderer::Renderer(int width, int height, GLFWwindow* window, bool debug):
	m_width{width},
	m_height{ height },
	m_window{window},
	m_debug{true}
{
	CreateInstance();
	CreateDevice();

	MakeDescriptorSetLayout();

	MakePipeline();
	FinalizeSetup();
	MakeAssets();
}


//reset and re-record command buffer usage mode
void Renderer::Render()
{
	m_device.waitForFences(1, &m_swapchainFrames[frameNumber].inFlight, VK_TRUE, UINT64_MAX);
	m_device.resetFences(1, &m_swapchainFrames[frameNumber].inFlight);

	//acquireNextImageKHR(vk::SwapChainKHR, timeout, semaphore_to_signal, fence)
	uint32_t imageIndex{ m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_swapchainFrames[frameNumber].imageAvailable, nullptr).value };

	vk::CommandBuffer commandBuffer = m_swapchainFrames[frameNumber].commandBuffer;

	commandBuffer.reset();

	PrepareFrame(imageIndex);

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
	m_device.destroyPipeline(m_pipeline);
	m_device.destroyPipelineLayout(m_pipelineLayout);
	m_device.destroyRenderPass(m_renderpass);
	m_device.destroyCommandPool(m_commandPool);
	m_device.destroyDescriptorSetLayout(descriptorSetLayout);
	m_device.destroyDescriptorPool(descriptorPool);
	

	delete triangleMesh;
			

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
	std::array<vk::Queue, 2> queues = vkInit::GetQueue
	(m_physicalDevice, m_device, m_surface, m_debug);
	m_graphicsQueue = queues[0];
	m_presentQueue = queues[1];
	MakeSwapchain();

}

void Renderer::MakeDescriptorSetLayout()
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

		descriptorSetLayout = vkInit::make_descriptor_set_layout(m_device, bindings);

}

void Renderer::MakePipeline()
{
	vkInit::GraphicsPipelineInBundle specification = {};
	specification.device = m_device;
	specification.vertexFilepath = "shaders/vertex.spv";
	specification.fragmentFilepath = "shaders/fragment.spv";
	specification.swapchainExtent = m_swapchainExtent;
	specification.swapchainImageFormat = m_swapchainFormat;
	specification.descriptorSetLayout = descriptorSetLayout;

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

	descriptorPool = vkInit::make_descriptor_pool(m_device, static_cast<uint32_t>(m_swapchainFrames.size()), bindings);

	for (vkUtil::SwapChainFrame& frame : m_swapchainFrames) 
	{
		frame.imageAvailable = vkInit::make_semaphore(m_device, m_debug);
		frame.renderFinished = vkInit::make_semaphore(m_device, m_debug);
		frame.inFlight = vkInit::make_fence(m_device, m_debug);

		frame.make_ubo_resources(m_device, m_physicalDevice);

		frame.descriptorSet = vkInit::allocate_descriptor_set(m_device, descriptorPool, descriptorSetLayout);

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

	vk::ClearValue clearColor = { std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);


	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_swapchainFrames[imageIndex].descriptorSet, nullptr);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
	
	PrepareScene(commandBuffer);

	commandBuffer.draw(3, 1, 0, 0);

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
	triangleMesh = new TriangleMesh(m_device, m_physicalDevice);

}



void Renderer::PrepareScene(vk::CommandBuffer commandBuffer)
{
	vk::Buffer vertexBuffers[] = { triangleMesh->vertexBuffer.buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
}

void Renderer::PrepareFrame(uint32_t imageIndex)
{
	glm::vec3 eye = { 1.0f, 0.0f, -1.0f };
	glm::vec3 center = { 0.0f, 0.0f, 0.0f };
	glm::vec3 up = { 0.0f, 0.0f, -1.0f };
	glm::mat4 view = glm::lookAt(eye, center, up);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height), 0.1f, 10.0f);
	projection[1][1] *= -1;

	m_swapchainFrames[imageIndex].cameraData.view = view;
	m_swapchainFrames[imageIndex].cameraData.projection = projection;
	m_swapchainFrames[imageIndex].cameraData.viewProjection = projection * view;
	
	
	memcpy(m_swapchainFrames[imageIndex].cameraDataWriteLocation, &(m_swapchainFrames[imageIndex].cameraData), sizeof(vkUtil::UBO));

	m_swapchainFrames[imageIndex].write_descriptor_set(m_device);
}




