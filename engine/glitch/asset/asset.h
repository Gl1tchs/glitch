/**
 * @file asset.h
 *
 */

namespace gl {

/**
 * Enforces that T has a static load() method accepting a path.
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

} //namespace gl