/**
 * @file material_definitions.h
 */

#pragma once

#include "glitch/renderer/material.h"

namespace gl {

GL_API MaterialDefinition get_unlit_material_definition(
		uint32_t p_msaa_samples);

GL_API MaterialDefinition get_urp_material_definition(uint32_t p_msaa_samples);

} //namespace gl