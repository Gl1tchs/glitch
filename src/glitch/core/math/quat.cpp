#include "glitch/core/math/quat.h"

Quat::Quat() : w(1), x(0), y(0), z(0) {}

Quat::Quat(const Quat& p_q) : w(p_q.w), x(p_q.x), y(p_q.y), z(p_q.z) {}
Quat::Quat(float p_x, float p_y, float p_z) : w(0), x(p_x), y(p_y), z(p_z) {}
Quat::Quat(float p_w, float p_x, float p_y, float p_z) :
		w(p_w), x(p_x), y(p_y), z(p_z) {}

Quat& Quat::operator=(const Quat& p_rhs) {
	w = p_rhs.w;
	x = p_rhs.x;
	y = p_rhs.y;
	z = p_rhs.z;
	return *this;
}

Quat Quat::from_euler(const Vec3f& p_euler) {
	float x = p_euler.x * 0.5;
	float y = p_euler.y * 0.5;
	float z = p_euler.z * 0.5;

	float cX = cosf(x);
	float cY = cosf(y);
	float cZ = cosf(z);

	float sX = sinf(x);
	float sY = sinf(y);
	float sZ = sinf(z);

	return Quat(cX * cY * cZ - sX * sY * sZ, sX * cY * cZ + sY * sZ * cX,
			sY * cX * cZ - sX * sZ * cY, sX * sY * cZ + sZ * cX * cY);
}

Quat Quat::from_axis_angle(Vec3f p_axis, float p_angle) {
	float half_angle = p_angle * 0.5;

	float sin_2 = sinf(half_angle);
	float cos_2 = cosf(half_angle);

	float sin_norm = sin_2 /
			sqrtf(p_axis.x * p_axis.x + p_axis.y * p_axis.y +
					p_axis.z * p_axis.z);

	return Quat(cos_2, p_axis.x * sin_norm, p_axis.y * sin_norm,
			p_axis.z * sin_norm);
}

float Quat::dot(const Quat& p_q) const {
	return w * p_q.w + x * p_q.x + y * p_q.y + z * p_q.z;
}

float Quat::norm() const { return sqrtf(norm_sq()); }

float Quat::norm_sq() const { return dot(*this); }

Quat Quat::normalize() {
	float i_len = 1 / norm();
	return *this * i_len;
}

Quat Quat::conjugate() const { return Quat(w, -x, -y, -z); }

void Quat::rotate_vec(float& p_vx, float& p_vy, float& p_vz) {
	// https://raw.org/proof/vector-rotation-using-quaternions/
	// t = 2q x v
	float tx = 2. * (y * p_vz - z * p_vy);
	float ty = 2. * (z * p_vx - x * p_vz);
	float tz = 2. * (x * p_vy - y * p_vx);

	// v + w t + q x t
	p_vx = p_vx + w * tx + y * tz - z * ty;
	p_vy = p_vy + w * ty + z * tx - x * tz;
	p_vz = p_vz + w * tz + x * ty - y * tx;
}

Quat& Quat::operator+=(const Quat& p_q) {
	w += p_q.w;
	x += p_q.x;
	y += p_q.y;
	z += p_q.z;
	return *this;
}

Quat& Quat::operator-=(const Quat& p_q) {
	w -= p_q.w;
	x -= p_q.x;
	y -= p_q.y;
	z -= p_q.z;
	return *this;
}

Quat& Quat::operator*=(float p_scale) {
	w *= p_scale;
	x *= p_scale;
	y *= p_scale;
	z *= p_scale;
	return *this;
}

Quat& Quat::operator*=(const Quat& p_q) {
	float w1 = w;
	float x1 = x;
	float y1 = y;
	float z1 = z;

	float w2 = p_q.w;
	float x2 = p_q.x;
	float y2 = p_q.y;
	float z2 = p_q.z;

	w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;
	x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
	y = w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2;
	z = w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2;

	return *this;
}

Quat Quat::operator-() const { return Quat(-w, -x, -y, -z); }

Quat Quat::operator*(const Quat& p_q) const { return Quat(*this) *= p_q; }

Quat Quat::operator*(float& p_scale) const {
	return Quat(w * p_scale, x * p_scale, y * p_scale, z * p_scale);
}

Vec3f Quat::operator*(const Vec3f& p_vec) const {
	// quaternion representation of the vector
	Quat vec_quat(0, p_vec.x, p_vec.y, p_vec.z);

	// apply the rotation: q * v * q⁻¹
	Quat res = (*this) * vec_quat * this->conjugate();

	return Vec3f(res.x, res.y, res.z);
}

Quat Quat::operator+(const Quat& p_q) const {
	const Quat& q1 = *this;
	return Quat(q1.w + p_q.w, q1.x + p_q.x, q1.y + p_q.y, q1.z + p_q.z);
}

Quat Quat::operator-(const Quat& p_q2) const {
	const Quat& q1 = *this;
	return Quat(q1.w - p_q2.w, q1.x - p_q2.x, q1.y - p_q2.y, q1.z - p_q2.z);
}
