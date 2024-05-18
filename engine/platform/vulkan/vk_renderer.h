#pragma once

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "renderer/renderer.h"

#include "core/deletion_queue.h"
#include "core/window.h"

#include "platform/vulkan/vk_buffer.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_pipeline.h"
#include "platform/vulkan/vk_swapchain.h"

constexpr uint32_t FRAME_OVERLAP = 2;

struct VulkanFrameData {
	VulkanCommandPool command_pool;
	VulkanCommandBuffer main_command_buffer;

	VkSemaphore swapchain_semaphore, render_semaphore;
	VkFence render_fence;

	DeletionQueue deletion_queue;
	// DescriptorAllocator frame_descriptors;
};

class VulkanRenderer : public Renderer {
public:
	VulkanRenderer(Ref<Window> window);
	virtual ~VulkanRenderer();

	static VulkanRenderer* get_instance();

	// TODO add a renderable tree ex.:
	// void push_renderable();

	void draw() override;

	void immediate_submit(
			std::function<void(VulkanCommandBuffer& cmd)>&& function);

private:
	void _geometry_pass(VulkanCommandBuffer& cmd);

	void _present_image(
			VulkanCommandBuffer& cmd, uint32_t swapchain_image_index);

private:
	static VulkanRenderer* s_instance;

	Ref<Window> window;

	// vulkan context
	VulkanContext context;

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
	VulkanCommandBuffer imm_command_buffer;
	VulkanCommandPool imm_command_pool;

	// temp
	VulkanImage texture_image{};
	VkSampler sampler_linear;

	VkDescriptorSetLayout descriptor_layout;
	VkDescriptorSet descriptor_set;
	VulkanDescriptorAllocator descriptor_allocator;

	VulkanPipelineLayout layout;
	VulkanPipeline pipeline;

	VulkanBuffer vertex_buffer;
	VulkanBuffer index_buffer;
	VkDeviceAddress vertex_buffer_address;

	struct Vertex {
		Vector3f position;
		float uv_x;
		Vector3f normal;
		float uv_y;
		Vector4f color;
	};

	struct PushConstants {
		VkDeviceAddress vertex_buffer;
	};
	// end temp

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
