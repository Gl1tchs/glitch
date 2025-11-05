/**
 * @file asset.h
 *
 */

namespace gl {

/**
 * Concept representing an asset type which has a static loader method
 * that returns the optional shared pointer type of the asset.
 *
 */
template <typename T, typename... Args>
concept AssetType = requires(const fs::path& p_path, Args&&... p_args) {
	{
		T::load(p_path, std::forward<Args>(p_args)...)
	} -> std::same_as<std::optional<std::shared_ptr<T>>>;
};

} //namespace gl