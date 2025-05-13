/**
 * @file matrix_clip_space.h
 *
 */

#pragma once

#include "glitch/core/math/matrix.h"

namespace gmath {

Mat4f ortho(float p_left, float p_right, float p_bottom, float p_top,
		float p_z_near, float p_z_far);

Mat4f perspective(float p_fovy, float p_aspect, float p_znear, float p_zfar);

Mat4f look_at(const Vec3f& p_eye, const Vec3f& p_center, const Vec3f& p_up);

} //namespace gmath