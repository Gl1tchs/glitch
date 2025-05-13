/**
 * @file quat.h
 */

#pragma once

#include "glitch/core/math/vector.h"

struct Quat {
	float w, x, y, z;

	Quat();
	Quat(const Quat& p_q);
	Quat(float p_x, float p_y, float p_z);
	Quat(float p_w, float p_x, float p_y, float p_z);

	Quat& operator=(const Quat& p_rhs);

	// Creates a quaternion from euler angles in the order of ZXY
	static Quat from_euler(const Vec3f& p_euler);

	static Quat from_axis_angle(Vec3f p_axis, float p_angle);

	float dot(const Quat& p_q) const;
	float norm() const;
	float norm_sq() const;

	Quat normalize();

	Quat conjugate() const;

	void rotate_vec(float& p_vx, float& p_vy, float& p_vz);

	Quat& operator+=(const Quat& p_q);
	Quat& operator-=(const Quat& p_q);
	Quat& operator*=(float p_scale);
	Quat& operator*=(const Quat& p_q);

	Quat operator-() const;
	Quat operator*(const Quat& p_q) const;
	Quat operator*(float& p_scale) const;
	Vec3f operator*(const Vec3f& p_vec) const;
	Quat operator+(const Quat& p_q) const;
	Quat operator-(const Quat& p_q) const;
};