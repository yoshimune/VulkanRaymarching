﻿#include "TriangleApp.h"

#include <fstream>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glm;
using namespace std;


// Public ===================================================================

// 準備
void TriangleApp::prepare()
{
	/* VERTEX */
	const vec3 red(1.0f, 0.0f, 0.0f);
	const vec3 green(0.0f, 1.0f, 0.0f);
	const vec3 blue(0.0f, 0.0f, 1.0f);

	Vertex vertices[] = {
		{ vec3(-1.0f, 1.0f, 0.0f), red },
		{ vec3(-1.0f, -1.0f, 0.0f), green},
		{ vec3(1.0f, 1.0f, 0.0f), blue },
		{ vec3(1.0f, -1.0f, 0.0f), red },
	};
	uint32_t indices[] = { 0,1,2, 2,1,3 };

	m_vertexBuffer = createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	m_indexBuffer = createBuffer(sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// 頂点データの書き込み
	{
		void* p;
		vkMapMemory(m_device, m_vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, vertices, sizeof(vertices));
		vkUnmapMemory(m_device, m_vertexBuffer.memory);
	}
	// インデックスデータの書き込み
	{
		void* p;
		vkMapMemory(m_device, m_indexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, indices, sizeof(indices));
		vkUnmapMemory(m_device, m_indexBuffer.memory);
	}
	m_indexCount = _countof(indices);


	// ユニフォームバッファ
	m_uniformBuffers.resize(m_swapchainViews.size());
	for (auto& v : m_uniformBuffers)
	{
		VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		v = createBuffer(sizeof(ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
	}

	prepareDescriptorSetLayout();
	prepareDescriptorPool();
	prepareDescriptorSet();


	// 頂点の入力設定
	VkVertexInputBindingDescription inputBinding{
		0,							// binding
		sizeof(Vertex),				// stride
		VK_VERTEX_INPUT_RATE_VERTEX	// inputRate
	};

	array<VkVertexInputAttributeDescription, 2> inputAttribs{
		{
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
		}
	};
	VkPipelineVertexInputStateCreateInfo vertexInputCI{};
	vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCI.vertexBindingDescriptionCount = 1;
	vertexInputCI.pVertexBindingDescriptions = &inputBinding;
	vertexInputCI.vertexAttributeDescriptionCount = uint32_t(inputAttribs.size());
	vertexInputCI.pVertexAttributeDescriptions = inputAttribs.data();


	/* ブレンディングの設定 */
	const auto colorWriteAll = \
		VK_COLOR_COMPONENT_R_BIT | \
		VK_COLOR_COMPONENT_G_BIT | \
		VK_COLOR_COMPONENT_B_BIT | \
		VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = VK_TRUE;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.colorWriteMask = colorWriteAll;
	VkPipelineColorBlendStateCreateInfo cbCI{};
	cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cbCI.attachmentCount = 1;
	cbCI.pAttachments = &blendAttachment;
	

	/* ビューポートの設定 */
	VkViewport viewport;
	{
		viewport.x = 0.0f;
		viewport.y = float(m_swapchainExtent.height);
		viewport.width = float(m_swapchainExtent.width);
		viewport.height = -1.0f * float(m_swapchainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
	}
	VkRect2D scissor = {
		{0,0},	// offset
		m_swapchainExtent
	};
	VkPipelineViewportStateCreateInfo viewportCI{};
	viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCI.viewportCount = 1;
	viewportCI.pViewports = &viewport;
	viewportCI.scissorCount = 1;
	viewportCI.pScissors = &scissor;
	

	// プリミティブトポロジー設定
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
	inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// ラスタライザステート
	VkPipelineRasterizationStateCreateInfo rasterizerCI{};
	rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCI.cullMode = VK_CULL_MODE_NONE;
	rasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCI.lineWidth = 1.0f;

	// マルチサンプル設定
	VkPipelineMultisampleStateCreateInfo multisampleCI{};
	multisampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// デプスステンシルステート設定
	VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
	depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCI.depthTestEnable = VK_TRUE;
	depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilCI.depthWriteEnable = VK_TRUE;
	depthStencilCI.stencilTestEnable = VK_FALSE;

	// シェーダーバイナリ読み込み
	vector<VkPipelineShaderStageCreateInfo> shaderStages
	{
		loadShaderModule("shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		loadShaderModule("shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	// パイプラインレイアウト
	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout);

	// パイプラインの構築
	VkGraphicsPipelineCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	ci.stageCount = uint32_t(shaderStages.size());
	ci.pStages = shaderStages.data();
	ci.pInputAssemblyState = &inputAssemblyCI;
	ci.pVertexInputState = &vertexInputCI;
	ci.pRasterizationState = &rasterizerCI;
	ci.pDepthStencilState = &depthStencilCI;
	ci.pMultisampleState = &multisampleCI;
	ci.pViewportState = &viewportCI;
	ci.pColorBlendState = &cbCI;
	ci.renderPass = m_renderPass;
	ci.layout = m_pipelineLayout;
	vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_pipeline);

	// ShaderModule はもう不要なので破棄
	for (const auto& v : shaderStages)
	{
		vkDestroyShaderModule(m_device, v.module, nullptr);
	}
}

// クリーンアップ
void TriangleApp::cleanup()
{
	for (auto& v : m_uniformBuffers)
	{
		vkDestroyBuffer(m_device, v.buffer, nullptr);
		vkFreeMemory(m_device, v.memory, nullptr);
	}

	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_pipeline, nullptr);

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

	vkFreeMemory(m_device, m_vertexBuffer.memory, nullptr);
	vkFreeMemory(m_device, m_indexBuffer.memory, nullptr);
	vkDestroyBuffer(m_device, m_vertexBuffer.buffer, nullptr);
	vkDestroyBuffer(m_device, m_indexBuffer.buffer, nullptr);
}

// コマンド作成
void TriangleApp::makeCommand(VkCommandBuffer command)
{
	// ユニフォームバッファの中身を更新する
	ShaderParameters shaderParam{};
	shaderParam.resolution = vec4( width, height, 0.0f, 0.0f);
	shaderParam.camera_pos = vec4( 0.0f, 0.0f, -4.0f, 0.0f);
	shaderParam.camera_dir = vec4( 0.0f, 0.0f, 1.0f, 0.0f);
	shaderParam.camera_up = vec4( 0.0f, 1.0f, 0.0f, 0.0f);
	shaderParam.camera_side = vec4(1.0f, 0.0f, 0.0f, 0.0f);

	auto rotation = glm::rotate(glm::identity<glm::mat4>(), glm::radians(float(45.0 * currentTime)), glm::vec3(0, 0, 1.0));
	auto translation = glm::translate(glm::identity<glm::mat4>(), vec3(0, 0, 3.0));
	//shaderParam.light_pos = vec4(1.0f, -2.5f, 1.0f, 0.0f);
	shaderParam.light_pos = translation * rotation * vec4(0.0f, -1.0f, -3.0f, 1.0f);

	//printf("%s \n", glm::to_string(rotation));

	printf("%f, %f, %f, %f \n", shaderParam.light_pos.x, shaderParam.light_pos.y, shaderParam.light_pos.z, shaderParam.light_pos.w);
	//printf("%f, %f, %f, %f \n", rotation[0].x, rotation[0].y, rotation[0].z, rotation[0].w);
	//printf("%f, %f, %f, %f \n", rotation[1].x, rotation[1].y, rotation[1].z, rotation[1].w);
	//printf("%f, %f, %f, %f \n", rotation[2].x, rotation[2].y, rotation[2].z, rotation[2].w);
	//printf("%f, %f, %f, %f \n", rotation[3].x, rotation[3].y, rotation[3].z, rotation[3].w);
	//printf("\n");

	shaderParam.light_color = vec4( 4.0f, 5.0f, 6.0f, 0.0f );
	{
		auto memory = m_uniformBuffers[m_imageIndex].memory;
		void* p;
		vkMapMemory(m_device, memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, &shaderParam, sizeof(shaderParam));
		vkUnmapMemory(m_device, memory);
	}

	// 作成したパイプラインをセット
	vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	// 各バッファオブジェクトのセット
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(command, 0, 1, &m_vertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(command, m_indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);

	// ディスクリプタセットをセット
	VkDescriptorSet descriptorSets[] = {
		m_descriptorSet[m_imageIndex]
	};
	vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, descriptorSets, 0, nullptr);

	// 三角形描画
	vkCmdDrawIndexed(command, m_indexCount, 1, 0, 0, 0);
}


// Private ==================================================================

// バッファオブジェクトを作成する
TriangleApp::BufferObject TriangleApp::createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
{
	BufferObject obj;
	VkBufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.usage = usage;
	ci.size = size;
	auto result = vkCreateBuffer(m_device, &ci, nullptr, &obj.buffer);
	checkResult(result);

	// メモリ量の算出
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements(m_device, obj.buffer, &reqs);
	VkMemoryAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.allocationSize = reqs.size;

	// メモリタイプの判定
	info.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits, flags);

	// メモリの確保
	vkAllocateMemory(m_device, &info, nullptr, &obj.memory);

	// メモリのバインド
	vkBindBufferMemory(m_device, obj.buffer, obj.memory, 0);
	return obj;
}

VkPipelineShaderStageCreateInfo TriangleApp::loadShaderModule(const char* fileName, VkShaderStageFlagBits stage)
{
	ifstream infile(fileName, std::ios::binary);
	if (!infile)
	{
		OutputDebugStringA("file not found.\n");
		DebugBreak();
	}
	vector<char> filedata;
	filedata.resize(uint32_t(infile.seekg(0, ifstream::end).tellg()));
	infile.seekg(0, ifstream::beg).read(filedata.data(), filedata.size());

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.pCode = reinterpret_cast<uint32_t*>(filedata.data());
	ci.codeSize = filedata.size();
	vkCreateShaderModule(m_device, &ci, nullptr, &shaderModule);

	VkPipelineShaderStageCreateInfo shaderStageCI{};
	shaderStageCI.stage = stage;
	shaderStageCI.module = shaderModule;
	shaderStageCI.pName = "main";
	return shaderStageCI;
}

void TriangleApp::prepareDescriptorSetLayout()
{
	vector<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayoutBinding bindingUBO{};
	bindingUBO.binding = 0;
	bindingUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindingUBO.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindingUBO.descriptorCount = 1;
	bindings.push_back(bindingUBO);

	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = uint32_t(bindings.size());
	ci.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(m_device, &ci, nullptr, &m_descriptorSetLayout);
}

void TriangleApp::prepareDescriptorPool()
{
	array<VkDescriptorPoolSize, 1> descPoolSize;
	descPoolSize[0].descriptorCount = 1;
	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.maxSets = uint32_t(m_uniformBuffers.size());
	ci.poolSizeCount = uint32_t(descPoolSize.size());
	ci.pPoolSizes = descPoolSize.data();
	vkCreateDescriptorPool(m_device, &ci, nullptr, &m_descriptorPool);
}

void TriangleApp::prepareDescriptorSet()
{
	vector<VkDescriptorSetLayout> layouts;
	for (int i = 0; i< int(m_uniformBuffers.size()); ++i)
	{
		layouts.push_back(m_descriptorSetLayout);
	}
	VkDescriptorSetAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = m_descriptorPool;
	ai.descriptorSetCount = uint32_t(m_uniformBuffers.size());
	ai.pSetLayouts = layouts.data();
	m_descriptorSet.resize(m_uniformBuffers.size());
	vkAllocateDescriptorSets(m_device, &ai, m_descriptorSet.data());

	// ディスクリプタセットへ書き込み
	for (int i = 0;i<int(m_uniformBuffers.size()); ++i)
	{
		VkDescriptorBufferInfo descUBO{};
		descUBO.buffer = m_uniformBuffers[i].buffer;
		descUBO.offset = 0;
		descUBO.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet ubo{};
		ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubo.dstBinding = 0;
		ubo.descriptorCount = 1;
		ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo.pBufferInfo = &descUBO;
		ubo.dstSet = m_descriptorSet[i];

		vector<VkWriteDescriptorSet> writeSets = {
			ubo
		};
		vkUpdateDescriptorSets(m_device, uint32_t(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}