#pragma once
#include "Config.h"
#include "Memory.h"

/**
	Holds a vertex buffer for a triangle mesh.
*/
class Mesh {
public:
	Mesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);
	~Mesh();
	Buffer vertexBuffer;
	Buffer indexBuffer;
private:
	vk::Device logicalDevice;
};