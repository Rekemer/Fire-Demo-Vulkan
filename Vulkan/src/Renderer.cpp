#include "Renderer.h"
#include "Instance.h"
#include "Logging.h"
#include "Device.h"
#include <glfw3.h>

Renderer::Renderer()
{
	BuildWindow();
	CreateInstance();
	CreateDevice();
	
}

Renderer::~Renderer()
{
	glfwTerminate();
	for (vkUtil::SwapChainFrame frame : m_swapchainFrames) {
		m_device.destroyImageView(frame.imageView);
	}
	m_device.destroySwapchainKHR(m_swapchain);
	m_device.destroy();
	m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
	m_instance.destroySurfaceKHR(m_surface);
	m_instance.destroy();

}

void Renderer::BuildWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);
}

void Renderer::CreateInstance()
{
	m_instance = vkInit::CreateInstance(m_debug, "Vulkan");
	m_dldi = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
	if (!m_debug) {
		return;
	}

	m_debugMessenger = vkInit::make_debug_messenger(m_instance, m_dldi);

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
	auto bundle = vkInit::CreateSwapchain(m_device,m_physicalDevice,m_surface,m_width,m_height,true);
	m_swapchain = bundle.swapchain;
	m_swapchainFrames = bundle.frames;
	m_swapchainFormat = bundle.format;
	m_swapchainExtent = bundle.extent;
}


