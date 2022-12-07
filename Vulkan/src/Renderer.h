#pragma once
#include "vulkan/vulkan.hpp"
#include"Frame.h"

class GLFWwindow;


class Renderer
{
public:
	Renderer();
	~Renderer();
private:
	void BuildWindow();
	void CreateInstance();
	void CreateDevice();

private:
	bool m_debug{true};
	int m_width{640};
	int m_height{480};
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
};

