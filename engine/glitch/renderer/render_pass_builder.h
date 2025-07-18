/**
 * @file render_pass_builder.h
 *
 */

#pragma once

#include "glitch/renderer/types.h"

namespace gl {

class RenderPassBuilder {
public:
	RenderPassBuilder();

	RenderPassBuilder& add_attachment(RenderPassAttachment p_attachment);

	RenderPassBuilder& add_color_attachment(DataFormat p_attachment);

	RenderPassBuilder& add_depth_attachment(DataFormat p_attachment);

	RenderPassBuilder& add_subpass(const SubpassInfo& p_attachments);

	RenderPass build();

private:
	std::vector<RenderPassAttachment> attachments;
	std::vector<SubpassInfo> subpasses;
};

} //namespace gl