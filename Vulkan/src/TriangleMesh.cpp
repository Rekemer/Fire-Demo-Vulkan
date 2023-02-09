#include "TriangleMesh.h"

Mesh::Mesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice) {

	this->logicalDevice = logicalDevice;

	std::vector<float> vertices = { {
		 -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f
	} };
	std::vector<uint32_t> indicies{ 0,1,2,2,3,0 };



	BufferInputChunk inputChunk;
	inputChunk.logicalDevice = logicalDevice;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.size = sizeof(float) * vertices.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eVertexBuffer;

	vertexBuffer = vkUtil::createBuffer(inputChunk);

	void* memoryLocation = logicalDevice.mapMemory(vertexBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertices.data(), inputChunk.size);
	logicalDevice.unmapMemory(vertexBuffer.bufferMemory);

		//make a staging buffer for indices
	inputChunk.size = sizeof(uint32_t) * indicies.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eIndexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible
		| vk::MemoryPropertyFlagBits::eHostCoherent;
	indexBuffer = vkUtil::createBuffer(inputChunk);

	//fill it with index data
	memoryLocation = logicalDevice.mapMemory(indexBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, indicies.data(), inputChunk.size);
	logicalDevice.unmapMemory(indexBuffer.bufferMemory);

}

Mesh::~Mesh() {

	logicalDevice.destroyBuffer(vertexBuffer.buffer);
	logicalDevice.freeMemory(vertexBuffer.bufferMemory);

	logicalDevice.destroyBuffer(indexBuffer.buffer);
	logicalDevice.freeMemory(indexBuffer.bufferMemory);


}