/**
 * @file matrix_transform.h
 *
 */

#include "glitch/core/math/matrix.h"

namespace gmath {

Mat4f translate(const Mat4f& p_mat, const Vec3f& p_translation);

Mat4f rotate(const Mat4f& p_mat, const float& p_angle, const Vec3f& p_axis);

Mat4f scale(const Mat4f& p_mat, const Vec3f& p_scale);

} //namespace gmath