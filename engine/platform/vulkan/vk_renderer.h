#pragma once

#include "renderer/renderer.h"

#include "core/deletion_queue.h"
#include "core/window.h"

#include "renderer/material.h"
#include "renderer/mesh.h"

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_image.h"
#include "platform/vulkan/vk_mesh.h"
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

	VkSemaphore swapchain_semaphore, render_semaphore;
	VkFence render_fence;

	DeletionQueue deletion_queue;
	VulkanDescriptorAllocator frame_descriptors;
};

class VulkanRenderer : public Renderer {
public:
	VulkanRenderer(Ref<Window> window);
	virtual ~VulkanRenderer();

	static VulkanRenderer* get_instance();

	void attach_camera(Camera* camera) override;

	void submit_mesh(Ref<Mesh> mesh, Ref<MaterialInstance> material) override;

	void draw() override;

	void wait_for_device() override;

	void immediate_submit(
			std::function<void(VulkanCommandBuffer& cmd)>&& function);

	static VulkanContext& get_context();

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

	Ref<VulkanImage> draw_image;
	Ref<VulkanImage> depth_image;
	VkExtent2D draw_extent{};
	float render_scale = 1.0f;

	VulkanFrameData frames[FRAME_OVERLAP];
	uint32_t frame_number{ 0 };

	// immediate commands
	VkFence imm_fence;
	VulkanCommandBuffer imm_command_buffer;
	VulkanCommandPool imm_command_pool;

	Camera* camera = nullptr;

	std::map<Ref<MaterialInstance>, std::vector<Ref<VulkanMesh>>>
			meshes_to_draw;

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
	void _init_descriptors();
	void _init_samplers();
};
