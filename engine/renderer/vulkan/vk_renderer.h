#pragma once

#include "renderer/renderer.h"

#include "core/deletion_queue.h"
#include "core/window.h"

#include "renderer/vulkan/vk_image.h"
#include "renderer/vulkan/vk_pch.h"
#include "renderer/vulkan/vk_swapchain.h"

constexpr uint32_t FRAME_OVERLAP = 2;

struct VulkanFrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;

	VkSemaphore swapchain_semaphore, render_semaphore;
	VkFence render_fence;

	DeletionQueue deletion_queue;
	// DescriptorAllocatorGrowable frame_descriptors;
};

class VulkanRenderer : public IRenderer {
public:
	VulkanRenderer(Ref<Window> window);
	virtual ~VulkanRenderer();

	static VulkanRenderer* get_instance();

	// TODO add a renderable tree ex.:
	// void push_renderable();

	void draw();

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

private:
	static VulkanRenderer* s_instance;

	Ref<Window> window;

	// vulkan context resources

	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice chosen_gpu;
	VkSurfaceKHR surface;
	VmaAllocator allocator;
	VkDebugUtilsMessengerEXT debug_messenger;

	VkQueue graphics_queue;
	uint32_t graphics_queue_family;

	// drawing resources

	Ref<VulkanSwapchain> swapchain;

	VulkanImage draw_image{};
	VulkanImage depth_image{};
	VkExtent2D draw_extent{};
	float render_scale = 1.0f;

	VulkanFrameData frames[FRAME_OVERLAP];
	uint32_t frame_number{ 0 };

	// immediate commands
	VkFence imm_fence;
	VkCommandBuffer imm_command_buffer;
	VkCommandPool imm_command_pool;

	DeletionQueue deletion_queue;

private:
	VulkanFrameData& get_current_frame() {
		return frames[frame_number % FRAME_OVERLAP];
	};

private:
	void _init_vulkan();
	void _init_swapchain();
	void _init_commands();
	void _init_sync_structures();
};
