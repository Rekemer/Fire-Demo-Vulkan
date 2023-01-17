#pragma once
#include "vulkan\vulkan.hpp"
#include <iostream>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include "glfw3.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

struct BufferInputChunk {
	size_t size;
	vk::BufferUsageFlagBits usage;
	vk::Device logicalDevice;
	vk::PhysicalDevice physicalDevice;
	vk::MemoryPropertyFlags memoryProperties;
};


struct Buffer {
	vk::Buffer buffer;
	vk::DeviceMemory bufferMemory;
};