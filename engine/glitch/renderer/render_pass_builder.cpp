#include "glitch/renderer/render_pass_builder.h"

#include "glitch/renderer/renderer.h"

namespace gl {

RenderPassBuilder::RenderPassBuilder() {}

RenderPassBuilder& RenderPassBuilder::add_attachment(RenderPassAttachment attachment) {
	_attachments.push_back(attachment);

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_color_attachment(DataFormat attachment) {
	_attachments.push_back({
			.format = attachment,
			.load_op = AttachmentLoadOp::CLEAR,
			.store_op = AttachmentStoreOp::STORE,
	});

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_depth_attachment(DataFormat attachment) {
	_attachments.push_back({
			.format = attachment,
			.load_op = AttachmentLoadOp::CLEAR,
			.store_op = AttachmentStoreOp::STORE,
			.is_depth_attachment = true,
	});

	return *this;
}

RenderPassBuilder& RenderPassBuilder::add_subpass(const SubpassInfo& attachments) {
	_subpasses.push_back(attachments);

	return *this;
}

RenderPass RenderPassBuilder::build() {
	auto device = Renderer::get_device();
	return device->render_pass_create(_attachments, _subpasses).value();
}

} //namespace gl