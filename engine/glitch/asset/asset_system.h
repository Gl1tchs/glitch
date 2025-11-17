/**
 * @file asset_system.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/core/uid.h"

namespace gl {

enum class PathProcessError {
	EMPTY_PATH,
	INVALID_IDENTIFIER,
	UNDEFINED_WORKING_DIR,
};

enum class AssetLoadingError {
	FILE_ERROR,
	PARSING_ERROR,
};

using AssetHandle = UID;

template <typename T> struct AssetRegistry {
private:
	inline static std::unordered_map<AssetHandle, std::shared_ptr<T>> s_map;
	friend class AssetSystem;
};

/**
 * Class representing the owner of the assets. By registering or loading assets
 * through the AssetSystem user can ensure that it will be deleted and acsessed safely.
 * User can call AssetSystem::collect_garbage to remove assets with no references.
 *
 */
class GL_API AssetSystem {
public:
	using AssetDeletionFn = std::function<void()>;

	static void init();

	static void shutdown();

	/**
	 * Remove unusued assets from the registry
	 *
	 */
	static void collect_garbage();

	/**
	 * Loads and registers the asset to the compatible asset registry.
	 *
	 */
	template <IsLoadableAsset T, typename... Args>
	static Result<AssetHandle, AssetLoadingError> load(std::string_view p_path, Args&&... p_args) {
		const auto absolute_path = get_absolute_path(p_path);
		if (absolute_path.has_error()) {
			return make_err<AssetHandle>(AssetLoadingError::FILE_ERROR);
		}

		const std::shared_ptr<T> asset = T::load(*absolute_path, std::forward<Args>(p_args)...);
		if (!asset) {
			return make_err<AssetHandle>(AssetLoadingError::PARSING_ERROR);
		}

		return register_asset(asset);
	}

	/**
	 * Creates and registers the asset to the compatible registry
	 *
	 */
	template <IsCreatableAsset T, typename... Args>
	static std::optional<AssetHandle> create(Args&&... p_args) {
		const std::shared_ptr<T> asset = T::create(std::forward<Args>(p_args)...);
		if (!asset) {
			return std::nullopt;
		}

		return register_asset(asset);
	}

	/**
	 * Retrieve asset from registry
	 *
	 */
	template <typename T> static std::shared_ptr<T> get(AssetHandle p_handle) {
		auto& registry = get_registry<T>();

		const auto it = registry.find(p_handle);
		if (it == registry.end()) {
			return nullptr;
		}

		return it->second;
	}

	/**
	 * Release given asset handle from registry.
	 *
	 */
	template <typename T> static bool free(AssetHandle p_handle) {
		auto& registry = get_registry<T>();

		const size_t num_removed = registry.erase(p_handle);
		return num_removed > 0;
	}

	/**
	 * Registers given asset to the registry, so that it would automatically deleted
	 * if no other reference is pointing to it.
	 *
	 */
	template <typename T> static AssetHandle register_asset(std::shared_ptr<T> p_asset) {
		AssetHandle handle; // new random uid
		// Push the asset to the register
		get_registry<T>().insert_or_assign(handle, p_asset);
		return handle;
	}

	template <typename T>
	static std::unordered_map<AssetHandle, std::shared_ptr<T>>& get_registry() {
		struct Registrar {
			Registrar() {
				AssetSystem::_get_cleanup_registry().push_back(
						[]() { AssetRegistry<T>::s_map.clear(); });

				AssetSystem::_get_gc_registry().push_back([]() {
					auto& map = AssetRegistry<T>::s_map;
					// Remove the elements, which has a use_count of 1
					// (only held and owned by the map)
					std::erase_if(map, [](const auto& item) {
						const auto& [handle, ptr] = item;
						return ptr.use_count() == 1;
					});
				});
			}
		};

		// type `Registrar` will only be initialized once, thus we can ensure the deletion of map
		// AssetRegistry<T>::s_map will only happen once.
		static Registrar s_registrar;

		return AssetRegistry<T>::s_map;
	}

	/**
	 * Transforms engine path format with suffix 'res://' to absolute path
	 *
	 */
	static Result<fs::path, PathProcessError> get_absolute_path(std::string_view p_path);

private:
	static std::vector<AssetDeletionFn>& _get_cleanup_registry();
	static std::vector<AssetDeletionFn>& _get_gc_registry();
};

} // namespace gl
