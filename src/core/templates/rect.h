#pragma once

#include "core/templates/vector.h"

template <typename T> struct Rect2D {
	Vec<T, 2> position;
	Vec<T, 2> size;
};

typedef Rect2D<int> Rect2i;
typedef Rect2D<uint32_t> Rect2u;
typedef Rect2D<float> Rect2f;
typedef Rect2D<double> Rect2d;
