#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#include <vector>

class VulkanAppBase
{
public:
	VulkanAppBase();
	virtual ~VulkanAppBase() {}

	void initialize(GLFWwindow* window, const char* appName);
	void terminate();

	virtual void render();

	// 以下、派生先で内容をオーバーライドする
	virtual void prepare() {}
	virtual void cleanup() {}
	virtual void makeCommand(VkCommandBuffer command) {}

protected:
	// 各処理メソッド（を書く予定）

	// 結果チェック
	static void checkResult(VkResult);

	// vkInstanceを初期化する
	void initializeInstance(const char* appName);

	//物理デバイスを取得する
	void selectPhysicalDevice();

	// デバイスキューインデックスを取得する
	uint32_t searchGraphicsQueueIndex();

	// 論理デバイスを作成する
	void createDevice();

	// コマンドプール準備
	void prepareCommandPool();

	// サーフェイスフォーマット設定
	void selectSurfaceFormat(VkFormat format);

	// スワップチェイン作成
	void createSwapChain(GLFWwindow* window);

	// デプスバッファ作成
	void createDepthBuffer();

	// VkImageView作成
	void createViews();

	// RenderPassの作成
	void createRenderPass();

	// Framebuffer作成
	void createFramebuffer();

	// コマンドバッファの作成
	void prepareCommandBuffers();

	// セマフォの用意
	void prepareSemaphores();

	// メモリタイプインデックスを取得
	uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps)const;

	// デバッグレポート有効化
	void enableDebugReport();

	// デバッグレポート無効化
	void disableDebugReport();


	// インスタンス
	VkInstance  m_instance;

	// 論理デバイス
	VkDevice m_device;

	// 物理デバイス
	VkPhysicalDevice m_physDev;

	// 物理デバイスのメモリプロパティ
	VkPhysicalDeviceMemoryProperties m_physMemProps;

	// Surface
	VkSurfaceKHR m_surface;

	// Surface format
	VkSurfaceFormatKHR m_surfaceFormat;

	// Surface Capabilities
	VkSurfaceCapabilitiesKHR  m_surfaceCaps;

	// デバイスキューインデックス
	uint32_t m_graphicsQueueIndex;
	VkQueue m_deviceQueue;

	// コマンドプール
	VkCommandPool m_commandPool;

	// Present
	VkPresentModeKHR m_presentMode;

	//Swapchain
	VkSwapchainKHR m_swapchain;

	// Swapchain extent
	VkExtent2D m_swapchainExtent;

	// Swapchain Images
	std::vector<VkImage> m_swapchainImages;

	// Swapchain Views
	std::vector<VkImageView> m_swapchainViews;

	// デプスバッファテクスチャ
	VkImage m_depthBuffer;

	// デプスバッファ デバイスメモリ
	VkDeviceMemory m_depthBufferMemory;

	// デプスバッファビュー
	VkImageView m_depthBufferView;

	// RenderPass
	VkRenderPass m_renderPass;

	// Framebuffer
	std::vector<VkFramebuffer> m_framebuffers;

	// Fence
	std::vector<VkFence> m_fences;

	// セマフォ
	VkSemaphore m_renderCompletedSem, m_presentCompletedSem;

	// デバッグレポート関連
	PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT m_vkDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT m_debugReport;

	// コマンドバッファ
	std::vector<VkCommandBuffer> m_commands;

	uint32_t m_imageIndex;

	int width;
	int height;
};

