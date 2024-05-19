#pragma once

#include "platform/vulkan/vk_commands.h"
#include "platform/vulkan/vk_context.h"
#include "platform/vulkan/vk_descriptors.h"
#include "platform/vulkan/vk_material.h"
#include "platform/vulkan/vk_mesh.h"
#include "renderer/mesh.h"
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

	void submit_mesh(Ref<Mesh> mesh /*, Ref<Material> material */) override;

	void draw() override;

	void immediate_submit(
			std::function<void(VulkanCommandBuffer& cmd)>&& function);

	static const VulkanContext& get_context();

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

	VulkanDescriptorAllocator descriptor_allocator;

	std::vector<VulkanMesh*> meshes_to_draw;

	// temp
	VulkanMetallicRoughnessMaterial metallic_roughness;
	VulkanMaterialInstance metallic_instance;

	VulkanImage white_image;
	VulkanImage texture_image;
	VkSampler sampler_linear;
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
