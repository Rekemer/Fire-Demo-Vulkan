#pragma once
#include"Config.h"
#include <glfw3.h>

namespace vkInit
{
	bool supported(std::vector<const char*>& extensions, std::vector<const char*>& layers, bool debug) 
	{
		//check extension support
		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

		if (debug) {
			std::cout << "Device can support the following extensions:\n";
			for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
				std::cout << '\t' << supportedExtension.extensionName << '\n';
			}
		}

		bool found;
		for (const char* extension : extensions) {
			found = false;
			for (vk::ExtensionProperties supportedExtension : supportedExtensions) {
				if (strcmp(extension, supportedExtension.extensionName) == 0) {
					found = true;
					if (debug) {
						std::cout << "Extension \"" << extension << "\" is supported!\n";
					}
				}
			}
			if (!found) {
				if (debug) {
					std::cout << "Extension \"" << extension << "\" is not supported!\n";
				}
				return false;
			}
		}

		//check layer support
		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

		if (debug) {
			std::cout << "Device can support the following layers:\n";
			for (vk::LayerProperties supportedLayer : supportedLayers) {
				std::cout << '\t' << supportedLayer.layerName << '\n';
			}
		}

		for (const char* layer : layers) {
			found = false;
			for (vk::LayerProperties supportedLayer : supportedLayers) {
				if (strcmp(layer, supportedLayer.layerName) == 0) {
					found = true;
					if (debug) {
						std::cout << "Layer \"" << layer << "\" is supported!\n";
					}
				}
			}
			if (!found) {
				if (debug) {
					std::cout << "Layer \"" << layer << "\" is not supported!\n";
				}
				return false;
			}
		}

		return true;
	}


	vk::Instance CreateInstance(bool debug,const  char* appName)
	{
		uint32_t version;
		vkEnumerateInstanceVersion(&version);

		auto appInfo = vk::ApplicationInfo
		{
			appName,
			version,
			"Vulkan",
			version,
			version,
		};
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		
		std::vector<const char*> extensions;
		
		
		for (auto  i = 0; i < glfwExtensionCount; i++) {
			extensions.emplace_back(glfwExtensions[i]);
		}
		if (debug) {
			extensions.push_back("VK_EXT_debug_utils");
		}
		if (debug) {
			std::cout << "extensions to be requested:\n";

			for (const char* extensionName : extensions) {
				std::cout << "\t\"" << extensionName << "\"\n";
			}
		}

		std::vector<const char*> layers;

		if (debug) {
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}
		
		if (!supported(extensions, layers, debug)) {
			return nullptr;
		}

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			layers.size(), layers.data(), // enabled layers
			extensions.size(), extensions.data() // enabled extensions
		);


		try {
			/*
			* from vulkan_funcs.h:
			*
			* createInstance( const VULKAN_HPP_NAMESPACE::InstanceCreateInfo &          createInfo,
					Optional<const VULKAN_HPP_NAMESPACE::AllocationCallbacks> allocator = nullptr,
					Dispatch const &                                          d = ::vk::getDispatchLoaderStatic())

			*/
			return vk::createInstance(createInfo);
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create Instance!\n";
			}
			return nullptr;
		}
		return nullptr;
	}
}