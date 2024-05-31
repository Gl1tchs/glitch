#pragma once

#include "gl/renderer/renderer.h"

#include "gl/core/deletion_queue.h"
#include "gl/core/timer.h"
#include "gl/core/window.h"

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_material.h"
#include "platform/vulkan/vk_swapchain.h"

constexpr uint32_t FRAME_OVERLAP = 2;

struct VulkanSceneData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_proj;
};

struct VulkanFrameData {
	VulkanCommandPool command_pool;
	VulkanCommandBuffer main_command_buffer;

	VkSemaphore image_available_semaphore, render_finished_semaphore;
	VkFence render_fence;

	DeletionQueue deletion_queue;
	VulkanDescriptorAllocator frame_descriptors;
};

class VulkanRenderer : public Renderer {
public:
	VulkanRenderer(Ref<Window> window);
	virtual ~VulkanRenderer();

	void wait_and_render() override;

	void wait_for_device() override;

	void immediate_submit(
			std::function<void(VulkanCommandBuffer& cmd)>&& function);

	static VulkanRenderer* get_instance();

	static VulkanContext& get_context();

private:
	void _record_commands(
			VulkanCommandBuffer& cmd, const uint32_t swapchain_image_index);

	void _submit_commands(VulkanCommandBuffer& cmd);

	void _present_image(
			VulkanCommandBuffer& cmd, uint32_t swapchain_image_index);

	void _geometry_pass(VulkanCommandBuffer& cmd);

	void _compute_pass(VulkanCommandBuffer& cmd);

	void _request_resize();

private:
	static VulkanRenderer* s_instance;

	Timer timer;
	struct ComputeGlobalUniformBuffer {
		float delta_time;
	};

	DeletionQueue deletion_queue;

	Ref<Window> window;

	// vulkan context
	VulkanContext context;

	// drawing resources
	Ref<VulkanSwapchain> swapchain;

	Ref<VulkanImage> draw_image;
	Ref<VulkanImage> depth_image;
	VkExtent2D draw_extent;

	// g buffer images
	Ref<VulkanImage> position_image;
	Ref<VulkanImage> normal_image;

	VulkanFrameData frames[FRAME_OVERLAP];
	uint32_t frame_number{ 0 };

	// immediate commands
	VkFence imm_fence;
	VulkanCommandBuffer imm_command_buffer;
	VulkanCommandPool imm_command_pool;

	// default data
	Ref<VulkanImage> white_image;
	Ref<VulkanImage> error_image;

	Ref<VulkanMetallicRoughnessMaterial> default_roughness;
	Ref<VulkanMaterialInstance> default_roughness_instance;

private:
	VulkanFrameData& get_current_frame() {
		return frames[frame_number % FRAME_OVERLAP];
	};

private:
	void _init_vulkan();
	void _init_swapchain();
	void _init_commands();
	void _init_sync_structures();
	void _init_descriptors();
	void _init_samplers();
	void _init_default_data();
};
