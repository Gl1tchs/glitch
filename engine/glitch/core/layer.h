/**
 * @file layer.h
 *
 */

#pragma once

namespace gl {

/**
 * Abstract class representing a layer for adding functionality to the engine.
 *
 */
class Layer {
public:
	virtual ~Layer() = default;

	virtual void start() = 0;

	virtual void update(float p_dt) = 0;

	virtual void destroy() = 0;
};

} // namespace gl
