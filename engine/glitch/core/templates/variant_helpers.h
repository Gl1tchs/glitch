/**
 * @file variant_helpers.h
 *
 */

#pragma once

namespace gl {

template <class... Ts> struct VariantOverloaded : Ts... {
	using Ts::operator()...;
};

} //namespace gl