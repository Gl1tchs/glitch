#pragma once

#include "renderer/renderer.h"

#include "core/deletion_queue.h"
#include "core/timer.h"
#include "core/window.h"

#include "backends/vulkan/vk_commands.h"
#include "backends/vulkan/vk_context.h"
#include "backends/vulkan/vk_descriptors.h"
#include "backends/vulkan/vk_image.h"
#include "backends/vulkan/vk_material.h"
#include "backends/vulkan/vk_swapchain.h"

constexpr uint32_t FRAME_OVERLAP = 2;

struct VulkanSceneData {
	glm::vec4 camera_pos;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_proj;
	glm::vec3 sun_direction;
	// in glsl this will be the w component
	// of the `sun_direction`
	float sun_power;
	glm::vec4 sun_color;
};

struct DrawPushConstants {
	glm::mat4 transform;
	VkDeviceAddress vertex_buffer;
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

	VulkanFrameData frames[FRAME_OVERLAP];
	uint32_t frame_number{ 0 };

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