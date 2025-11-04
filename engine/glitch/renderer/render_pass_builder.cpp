#include "glitch/renderer/render_pass_builder.h"

#include "glitch/renderer/render_backend.h"
#include "glitch/renderer/renderer.h"

namespace gl {

RenderPassBuilder::RenderPassBuilder() {}

RenderPassBuilder& RenderPassBuilder::add_attachment(RenderPassAttachment p_attachment) {
	attachments.push_back(p_attachment);

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_color_attachment(DataFormat p_attachment) {
	attachments.push_back({
			.format = p_attachment,
			.load_op = AttachmentLoadOp::CLEAR,
			.store_op = AttachmentStoreOp::STORE,
	});

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_depth_attachment(DataFormat p_attachment) {
	attachments.push_back({
			.format = p_attachment,
			.load_op = AttachmentLoadOp::CLEAR,
			.store_op = AttachmentStoreOp::STORE,
			.is_depth_attachment = true,
	});

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_subpass(const SubpassInfo& p_attachments) {
	subpasses.push_back(p_attachments);

	return *this;
}

RenderPass RenderPassBuilder::build() {
	std::shared_ptr<RenderBackend> backend = Renderer::get_backend();
	return backend->render_pass_create(attachments, subpasses);
}

} //namespace gl