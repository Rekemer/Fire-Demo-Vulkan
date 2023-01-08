#pragma once
#include "vulkan/vulkan.hpp"
#include"Frame.h"

class GLFWwindow;


class Renderer
{
public:
	Renderer(int width, int height, GLFWwindow* window,bool debug);
	~Renderer();
	void Render();
private:
	void BuildWindow();
	void CreateInstance();
	void CreateDevice();
	void MakePipeline();

	void FinalizeSetup();

	void RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

private:
	bool m_debug;
	int m_width;
	int m_height;
	GLFWwindow* m_window;
	
	//instance-related variables
	vk::Instance m_instance;

	//debug callback
	vk::DebugUtilsMessengerEXT m_debugMessenger;

	//dynamic instance dispatcher
	vk::DispatchLoaderDynamic m_dldi;

	vk::SurfaceKHR m_surface;

	//device-related variables
	vk::PhysicalDevice m_physicalDevice{ nullptr };
	vk::Device m_device{ nullptr };
	vk::Queue m_graphicsQueue{ nullptr };
	vk::Queue m_presentQueue{ nullptr };

	//swapchain
	vk::SwapchainKHR m_swapchain{ nullptr };
	std::vector<vkUtil::SwapChainFrame> m_swapchainFrames;
	vk::Format m_swapchainFormat;
	vk::Extent2D m_swapchainExtent;

	//pipeline-related variables
	vk::PipelineLayout m_pipelineLayout;
	vk::RenderPass m_renderpass;
	vk::Pipeline m_pipeline;


	//Command-related variables
	vk::CommandPool m_commandPool;
	vk::CommandBuffer m_mainCommandBuffer;

	//Synchronization objects
	vk::Fence inFlightFence;
	vk::Semaphore imageAvailable, renderFinished;


};

