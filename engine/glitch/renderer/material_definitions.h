/**
 * @file material_definitions.h
 */

#pragma once

#include "glitch/renderer/material.h"

namespace gl {

GL_API MaterialDefinition get_unlit_standard_definition(uint32_t p_msaa_samples,
		DataFormat p_color_attachment, DataFormat p_depth_attachment);

GL_API MaterialDefinition get_pbr_standard_definition(uint32_t p_msaa_samples,
		DataFormat p_color_attachment, DataFormat p_depth_attachment);

} //namespace gl