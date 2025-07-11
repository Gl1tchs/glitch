#include "glitch/renderer/render_pass_builder.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

RenderPassBuilder::RenderPassBuilder() {}

RenderPassBuilder& RenderPassBuilder::add_attachment(
		RenderPassAttachment p_attachment) {
	attachments.push_back(p_attachment);

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_color_attachment(
		DataFormat p_attachment) {
	attachments.push_back({
			.format = p_attachment,
			.load_op = ATTACHMENT_LOAD_OP_CLEAR,
			.store_op = ATTACHMENT_STORE_OP_STORE,
	});

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_depth_attachment(
		DataFormat p_attachment) {
	attachments.push_back({
			.format = p_attachment,
			.load_op = ATTACHMENT_LOAD_OP_CLEAR,
			.store_op = ATTACHMENT_STORE_OP_STORE,
			.is_depth_attachment = true,
	});

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_subpass(
		const SubpassInfo& p_attachments) {
	subpasses.push_back(p_attachments);

	return *this;
}

RenderPass RenderPassBuilder::build() {
	Ref<RenderBackend> backend = Renderer::get_backend();
	return backend->render_pass_create(attachments, subpasses);
}
