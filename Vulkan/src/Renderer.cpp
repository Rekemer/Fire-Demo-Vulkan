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
	m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
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
}
void Renderer::CreateDevice()
{
	m_physicalDevice = vkInit::ChoosePhysicalDevice(m_instance, m_debug);
}


