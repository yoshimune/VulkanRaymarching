﻿#pragma once

#include "../common/VulkanAppBase.h"
#include "glm/glm.hpp"

class TriangleApp : public VulkanAppBase
{
public:
	TriangleApp() : VulkanAppBase() {}

	virtual void prepare() override;
	virtual void cleanup() override;

	virtual void makeCommand(VkCommandBuffer command) override;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

private:
	// バッファを管理するオブジェクト
	struct BufferObject
	{
		VkBuffer buffer;		// バッファ
		VkDeviceMemory memory;	// デバイスメモリオブジェクトのOpaqueハンドル
	};

	struct ShaderParameters
	{
		glm::vec2 resolution;
		glm::vec3 camera_pos;
		glm::vec3 camera_dir;
		glm::vec3 camera_up;
		glm::vec3 camera_side;
	};

	BufferObject createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
	VkPipelineShaderStageCreateInfo loadShaderModule(const char* fileName, VkShaderStageFlagBits stage);

	void prepareDescriptorSetLayout();
	void prepareDescriptorPool();
	void prepareDescriptorSet();

	BufferObject m_vertexBuffer;
	BufferObject m_indexBuffer;
	std::vector<BufferObject> m_uniformBuffers;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSet;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;
	uint32_t m_indexCount;
};