#include "glitch/platform/vulkan/vk_backend.h"

#include <vulkan/vulkan_core.h>

RenderPass VulkanRenderBackend::render_pass_create(
		VectorView<RenderPassAttachment> p_attachments,
		VectorView<SubpassInfo> p_subpasses) {
	std::vector<VkAttachmentDescription> vk_attachments;
	for (const auto& attachment : p_attachments) {
		VkAttachmentDescription vk_attachment = {};
		vk_attachment.format = static_cast<VkFormat>(attachment.format);
		vk_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		vk_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		vk_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		vk_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vk_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vk_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vk_attachment.finalLayout = attachment.is_depth_attachment
				? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		vk_attachments.push_back(vk_attachment);
	}

	std::vector<std::vector<VkAttachmentReference>> vk_color_attachment_refs;
	std::vector<std::vector<VkAttachmentReference>> vk_input_attachment_refs;
	std::vector<VkAttachmentReference> vk_depth_attachment_refs;

	std::vector<VkSubpassDescription> vk_subpasses;
	for (const auto& subpass : p_subpasses) {
		std::vector<VkAttachmentReference> color_refs;
		std::vector<VkAttachmentReference> input_refs;
		std::optional<VkAttachmentReference> depth_ref;

		for (const auto& attachment : subpass.attachments) {
			VkAttachmentReference ref = {};
			ref.attachment = attachment.attachment_index;

			switch (attachment.type) {
				case SUBPASS_ATTACHMENT_COLOR:
					ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					color_refs.push_back(ref);
					break;
				case SUBPASS_ATTACHMENT_INPUT:
					ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					input_refs.push_back(ref);
					break;
				case SUBPASS_ATTACHMENT_DEPTH_STENCIL:
					ref.layout =
							VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					depth_ref = ref;
					break;
			}
		}

		// Push ref containers to keep them alive
		vk_color_attachment_refs.push_back(std::move(color_refs));
		vk_input_attachment_refs.push_back(std::move(input_refs));
		if (depth_ref) {
			vk_depth_attachment_refs.push_back(*depth_ref);
		}

		VkSubpassDescription vk_subpass = {};
		vk_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vk_subpass.colorAttachmentCount =
				vk_color_attachment_refs.back().size();
		vk_subpass.pColorAttachments = vk_color_attachment_refs.back().data();
		vk_subpass.inputAttachmentCount =
				vk_input_attachment_refs.back().size();
		vk_subpass.pInputAttachments = vk_input_attachment_refs.back().data();
		if (depth_ref) {
			vk_subpass.pDepthStencilAttachment =
					&vk_depth_attachment_refs.back();
		}

		vk_subpasses.push_back(vk_subpass);
	}

	std::vector<VkSubpassDependency> vk_dependencies;
	for (uint32_t i = 0; i + 1 < vk_subpasses.size(); ++i) {
		VkSubpassDependency dep = {};
		dep.srcSubpass = i;
		dep.dstSubpass = i + 1;
		dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		vk_dependencies.push_back(dep);
	}

	VkRenderPassCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.attachmentCount = vk_attachments.size();
	create_info.pAttachments = vk_attachments.data();
	create_info.subpassCount = vk_subpasses.size();
	create_info.pSubpasses = vk_subpasses.data();
	create_info.dependencyCount = vk_dependencies.size();
	create_info.pDependencies = vk_dependencies.data();

	VkRenderPass vk_render_pass;
	VK_CHECK(
			vkCreateRenderPass(device, &create_info, nullptr, &vk_render_pass));

	// Bookkeeping
	VulkanRenderPass* render_pass_info =
			VersatileResource::allocate<VulkanRenderPass>(resources_allocator);
	render_pass_info->vk_render_pass = vk_render_pass;

	// Populate clear colors
	render_pass_info->clear_values.resize(p_attachments.size());
	for (size_t i = 0; i < p_attachments.size(); ++i) {
		auto& clear = render_pass_info->clear_values[i];
		const auto& attachment = p_attachments[i];

		if (attachment.is_depth_attachment) {
			clear.depthStencil = { 1.0f, 0 };
		} else {
			clear.color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
		}
	}

	return RenderPass(render_pass_info);
}

void VulkanRenderBackend::render_pass_destroy(RenderPass p_render_pass) {
	VulkanRenderPass* render_pass_info = (VulkanRenderPass*)p_render_pass;

	vkDestroyRenderPass(device, render_pass_info->vk_render_pass, nullptr);

	VersatileResource::free(resources_allocator, render_pass_info);
}

FrameBuffer VulkanRenderBackend::frame_buffer_create(RenderPass p_render_pass,
		VectorView<Image> p_attachments, const glm::uvec2& p_extent) {
	VulkanRenderPass* render_pass_info = (VulkanRenderPass*)p_render_pass;

	std::vector<VkImageView> vk_attachments;
	for (const auto& attachment : p_attachments) {
		VulkanImage* vk_image = (VulkanImage*)attachment;
		vk_attachments.push_back(vk_image->vk_image_view);
	}

	VkFramebufferCreateInfo frame_buffer_info = {};
	frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frame_buffer_info.renderPass = render_pass_info->vk_render_pass;
	frame_buffer_info.attachmentCount = vk_attachments.size();
	frame_buffer_info.pAttachments = vk_attachments.data();
	frame_buffer_info.width = p_extent.x;
	frame_buffer_info.height = p_extent.y;
	frame_buffer_info.layers = 1;

	VkFramebuffer frame_buffer;
	VK_CHECK(vkCreateFramebuffer(
			device, &frame_buffer_info, nullptr, &frame_buffer));

	return FrameBuffer(frame_buffer);
}

void VulkanRenderBackend::frame_buffer_destroy(FrameBuffer p_frame_buffer) {
	vkDestroyFramebuffer(device, (VkFramebuffer)p_frame_buffer, nullptr);
}