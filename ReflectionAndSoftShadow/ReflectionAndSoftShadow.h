#pragma once

#include "../common/VulkanAppBase.h"
#include "glm/glm.hpp"


class ReflectionAndSoftShadow : public VulkanAppBase
{
public:
	ReflectionAndSoftShadow() : VulkanAppBase() {}

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

	struct UniformBufferObject
	{
		BufferObject shaderParameters;
		BufferObject shaderMaterials;
	};

	struct ShaderParameters
	{
		glm::vec4 resolution;
		glm::vec4 camera_pos;
		glm::vec4 camera_dir;
		glm::vec4 camera_up;
		glm::vec4 camera_side;
		glm::vec4 light_dir;
		glm::vec4 light_color;
		glm::vec4 sky_color_light;
		glm::vec4 sky_color;
	};
	struct ShaderMaterials
	{
		glm::vec4 sphere;
		glm::vec4 box;
		glm::vec4 torus_pos;
		glm::vec4 torus_size;
		glm::vec4 hexPrizm_pos;
		glm::vec4 hexPrizm_size;
		glm::vec4 octahedron;
	};


	const glm::vec3 lightBlue = glm::vec3(0.7f, 0.8f, 0.99f);
	const glm::vec3 blue = glm::vec3(0.1f, 0.1f, 0.6f);

	void prepareGeometry();
	void prepareUniformBuffer();
	ShaderParameters createShaderParameters();
	ShaderMaterials createShaderMaterials();

	BufferObject createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
	VkPipelineShaderStageCreateInfo loadShaderModule(const char* fileName, VkShaderStageFlagBits stage);

	void createAlphaPipelineInfo(
		std::vector<VkPipelineShaderStageCreateInfo>* shaderStages,
		VkPipelineDepthStencilStateCreateInfo* depthStencilCI,
		VkPipelineColorBlendAttachmentState* blendAttachment,
		VkPipelineColorBlendStateCreateInfo* cbCI);

	void prepareDescriptorSetLayout();
	void prepareDescriptorPool();
	void prepareDescriptorSet();

	BufferObject m_vertexBuffer;
	BufferObject m_indexBuffer;
	std::vector<UniformBufferObject> m_uniformBuffers;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSet;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline_alpha;
	uint32_t m_indexCount;
};