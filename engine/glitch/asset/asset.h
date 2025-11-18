/**
 * @file asset.h
 *
 */

#pragma once

namespace gl {

/**
 * Enforces that T has a static load() method accepting a path and other args.
 */
template <typename T, typename... Args>
concept IsLoadableAsset = requires(const fs::path& p_path) {
	{ T::load(p_path, std::declval<Args>()...) } -> std::same_as<std::shared_ptr<T>>;
};

/**
 * Enforces that T has a static create() method accepting arguments.
 */
template <typename T, typename... Args>
concept IsCreatableAsset = requires {
	{ T::create(std::declval<Args>()...) } -> std::same_as<std::shared_ptr<T>>;
};

template <typename T>
concept IsReflectedAsset = requires {
	{ T::get_type_name() } -> std::same_as<const char*>;
};

#define GL_REFLECT_ASSET(x)                                                                        \
	static constexpr const char* get_type_name() { return #x; }

} //namespace gl
