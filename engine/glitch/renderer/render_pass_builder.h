/**
 * @file render_pass_builder.h
 *
 */

#pragma once

#include <glgpu/glgpu.h>

#include <vector>

namespace gl {

class RenderPassBuilder {
public:
	RenderPassBuilder();

	RenderPassBuilder& add_attachment(RenderPassAttachment attachment);

	RenderPassBuilder& add_color_attachment(DataFormat attachment);

	RenderPassBuilder& add_depth_attachment(DataFormat attachment);

	RenderPassBuilder& add_subpass(const SubpassInfo& attachments);

	RenderPass build();

private:
	std::vector<RenderPassAttachment> _attachments;
	std::vector<SubpassInfo> _subpasses;
};

} //namespace gl