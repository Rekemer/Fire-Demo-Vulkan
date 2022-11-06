#pragma once
#include "vulkan/vulkan.hpp"
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

	vk::Instance m_instance;

	//debug callback
	vk::DebugUtilsMessengerEXT m_debugMessenger;

	//dynamic instance dispatcher
	vk::DispatchLoaderDynamic m_dldi;

	//device-related variables
	vk::PhysicalDevice m_physicalDevice{ nullptr };


};

