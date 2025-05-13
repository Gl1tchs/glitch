#include "glitch/core/math/matrix_clip_space.h"

namespace gmath {

Mat4f ortho(float p_left, float p_right, float p_bottom, float p_top,
		float p_znear, float p_zfar) {
	Mat4f result(1);
	result[0][0] = 2.0f / (p_right - p_left);
	result[1][1] = 2.0f / (p_top - p_bottom);
	result[2][2] = 1.0f / (p_zfar - p_znear);
	result[3][0] = -(p_right + p_left) / (p_right - p_left);
	result[3][1] = -(p_top + p_bottom) / (p_top - p_bottom);
	result[3][2] = -p_znear / (p_zfar - p_znear);
	return result;
}

Mat4f perspective(float p_fovy, float p_aspect, float p_znear, float p_zfar) {
	GL_ASSERT(abs(p_aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

	float const tan_half_fovy = tan(p_fovy / static_cast<float>(2));

	Mat4f result(0.0f);
	result[0][0] = 1.0f / (p_aspect * tan_half_fovy);
	result[1][1] = 1.0f / (tan_half_fovy);
	result[2][2] = p_zfar / (p_zfar - p_znear);
	result[2][3] = 1.0f;
	result[3][2] = -(p_zfar * p_znear) / (p_zfar - p_znear);

	return result;
}

Mat4f look_at(const Vec3f& p_eye, const Vec3f& p_center, const Vec3f& p_up) {
	const Vec3f f((p_center - p_eye).normalize());
	const Vec3f s((f.cross(p_up)).normalize());
	const Vec3f u(s.cross(f));

	Mat4f result(1);
	result[0][0] = s.x;
	result[1][0] = s.y;
	result[2][0] = s.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = -f.x;
	result[1][2] = -f.y;
	result[2][2] = -f.z;
	result[3][0] = -s.dot(p_eye);
	result[3][1] = -u.dot(p_eye);
	result[3][2] = f.dot(p_eye);

	return result;
}

} //namespace gmath