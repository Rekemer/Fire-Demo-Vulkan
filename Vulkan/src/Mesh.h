#pragma once
#include "Config.h"

namespace vkMesh {

	struct Vertex
	{

	};

	/**
		\returns the input binding description for a (vec2 pos, vec3 color) vertex format.
	*/
	vk::VertexInputBindingDescription getPosColorBindingDescription() {

		/* Provided by VK_VERSION_1_0
		typedef struct VkVertexInputBindingDescription {
			uint32_t             binding;
			uint32_t             stride;
			VkVertexInputRate    inputRate;
		} VkVertexInputBindingDescription;
		*/

		vk::VertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = 7 * sizeof(float);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		return bindingDescription;
	}

	/**
		\returns the input attribute descriptions for a (vec2 pos, vec3 color) vertex format.
	*/
	std::vector<vk::VertexInputAttributeDescription> getPosColorAttributeDescriptions() {

		/* Provided by VK_VERSION_1_0
		typedef struct VkVertexInputAttributeDescription {
			uint32_t    location;
			uint32_t    binding;
			VkFormat    format;
			uint32_t    offset;
		} VkVertexInputAttributeDescription;
		*/

		std::vector<vk::VertexInputAttributeDescription> attributes;
		attributes.resize(3);

		//Pos
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = vk::Format::eR32G32Sfloat;
		attributes[0].offset = 0;

		//Color
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = vk::Format::eR32G32B32Sfloat;
		attributes[1].offset = 2 * sizeof(float);

		//Color
		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].format = vk::Format::eR32G32Sfloat;
		attributes[2].offset = 5 * sizeof(float);

		return attributes;
	}
}