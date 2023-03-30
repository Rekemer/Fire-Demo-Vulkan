#pragma once
#include "vulkan/vulkan.hpp"
#include"Frame.h"
#include "TriangleMesh.h"
#include "Image.h"
#include "QueueFamilies.h"
class GLFWwindow;
class ImGui_ImplVulkanH_Window;

class Renderer
{
public:
	Renderer(int width, int height, GLFWwindow* window,bool debug);
	~Renderer();
	void Render(float time, float deltaTime);
	void ImguiInit();
private:
	void BuildWindow();
	void CreateInstance();
	void CreateDevice();
	void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);

	void MakeDescriptorSetLayouts();
	void MakePipeline();

	void FinalizeSetup();

	void MakeSwapchain();
	void RecreateSwapchain();
	void DestroySwapchain();

	void MakeFramebuffers();
	void MakeFrameResources();



	void RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

	//asset creation
	void MakeAssets();

	void PrepareScene(vk::CommandBuffer commandBuffer);
	void PrepareFrame(uint32_t imageIndex,float time, float deltaTime);

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
	vkUtil::QueueFamilyIndices m_queueIndicies;
	//swapchain
	vk::SwapchainKHR m_swapchain{ nullptr };
	std::vector<vkUtil::SwapChainFrame> m_swapchainFrames;
	vk::Format m_swapchainFormat;
	vk::Extent2D m_swapchainExtent;

	//pipeline-related variables
	vk::PipelineLayout m_pipelineLayout;
	vk::RenderPass m_renderpass;
	vk::RenderPass m_ImGuiRenderpass;
	vk::Pipeline m_pipeline;


	//Command-related variables
	vk::CommandPool m_commandPool;
	vk::CommandBuffer m_mainCommandBuffer;

	//Synchronization objects
	//vk::Fence inFlightFence;
	//vk::Semaphore imageAvailable, renderFinished;
	size_t maxFramesInFlight, frameNumber;


	//asset pointers
	Mesh* triangleMesh;

	//descriptor-related variables
	

	//descriptor-related variables
	vk::DescriptorSetLayout frameSetLayout;
	vk::DescriptorPool frameDescriptorPool; //Descriptors bound on a "per frame" basis
	vk::DescriptorSetLayout meshSetLayout;
	vk::DescriptorPool meshDescriptorPool; //Descriptors bound on a "per mesh" basis

	vk::DescriptorSet descriptorSetTextures;



	vkImage::Texture* textureNoise;
	vkImage::Texture* textureFlame;
	vkImage::Texture* textureFlameColor;


};

