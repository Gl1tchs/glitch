#include "glitch/core/math/matrix_transform.h"

namespace gmath {

Mat4f translate(const Mat4f& p_mat, const Vec3f& p_translation) {
	Mat4f result(p_mat);
	result[3] = p_mat[0] * p_translation[0] + p_mat[1] * p_translation[1] +
			p_mat[2] * p_translation[2] + p_mat[3];
	return result;
}

Mat4f rotate(const Mat4f& p_mat, const float& p_angle, const Vec3f& p_vec) {
	const float a = p_angle;
	const float c = cos(a);
	const float s = sin(a);

	Vec3f axis(p_vec.normalize());
	Vec3f temp(axis * (1.0f - c));

	Mat4f rotate;
	rotate[0][0] = c + temp[0] * axis[0];
	rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	rotate[1][1] = c + temp[1] * axis[1];
	rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	rotate[2][2] = c + temp[2] * axis[2];

	Mat4f result;
	result[0] = p_mat[0] * rotate[0][0] + p_mat[1] * rotate[0][1] +
			p_mat[2] * rotate[0][2];
	result[1] = p_mat[0] * rotate[1][0] + p_mat[1] * rotate[1][1] +
			p_mat[2] * rotate[1][2];
	result[2] = p_mat[0] * rotate[2][0] + p_mat[1] * rotate[2][1] +
			p_mat[2] * rotate[2][2];
	result[3] = p_mat[3];
	return result;
}

Mat4f scale(const Mat4f& p_mat, const Vec3f& p_scale) {
	Mat4f result;
	result[0] = p_mat[0] * p_scale[0];
	result[1] = p_mat[1] * p_scale[1];
	result[2] = p_mat[2] * p_scale[2];
	result[3] = p_mat[3];
	return result;
}

} //namespace gmath