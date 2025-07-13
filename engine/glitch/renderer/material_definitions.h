/**
 * @file material_definitions.h
 */

#pragma once

#include "glitch/renderer/material.h"

GL_API MaterialDefinition get_unlit_material_definition(
		ImageSamples p_msaa_samples);

GL_API MaterialDefinition get_urp_material_definition(
		ImageSamples p_msaa_samples);
