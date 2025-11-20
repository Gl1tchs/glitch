/**
 * @file asset_system.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/core/uid.h"

namespace gl {

using AssetHandle = UID;

struct AssetMetadata {
	const char* type_name;
	std::string path;

	/**
	 * A memory asset is an asset, created by engine or other resources,
	 * that does not depend on a file.
	 *
	 */
	bool is_memory_asset() const;
};

struct IAssetRegistry {
	virtual ~IAssetRegistry() = default;

	virtual size_t get_asset_size() const = 0;

	virtual void collect_garbage() = 0;

	virtual void clear() = 0;
	virtual void clear_non_persistent() = 0;

	virtual std::unordered_map<AssetHandle, AssetMetadata> get_asset_metadata() const = 0;

	virtual void reload_all() = 0;

	virtual void serialize(json& p_out_json) const = 0;
	virtual void deserialize(const json& p_in_json) = 0;
};

template <IsReflectedAsset T> struct AssetRegistry : public IAssetRegistry {
	struct AssetEntry {
		std::shared_ptr<T> instance;
		std::string path;
		bool is_persistent = false;
	};

	std::unordered_map<AssetHandle, AssetEntry> assets;

	virtual ~AssetRegistry() = default;

	static AssetRegistry& get();

	size_t get_asset_size() const override;

	void collect_garbage() override;

	/**
	 * Registers given asset to the registry, so that it would automatically deleted
	 * if no other reference is pointing to it.
	 *
	 * @param p_path Path of the asset to be registered
	 * @param p_prev_handle Handle of the asset to be registered. Default is a random UID
	 */
	AssetHandle register_asset(std::shared_ptr<T> p_asset, const std::string& p_path,
			std::optional<AssetHandle> p_prev_handle = std::nullopt);

	/**
	 * Registers an asset that will live for the lifetime of the application.
	 * Garbage collection will skip this asset even if the reference count is 1.
	 */
	AssetHandle register_asset_persistent(
			std::shared_ptr<T> p_asset, const std::string& p_path = "");

	std::shared_ptr<T> get_asset(AssetHandle p_handle);

	std::shared_ptr<T> get_asset_by_path(const std::string& p_path);

	std::optional<AssetHandle> get_handle_by_path(const std::string& p_path) const;

	std::optional<AssetMetadata> get_metadata(AssetHandle p_handle);

	bool erase(AssetHandle p_handle);

	void clear() override;
	void clear_non_persistent() override;

	std::unordered_map<AssetHandle, AssetMetadata> get_asset_metadata() const override;

	void reload_all() override;

	void serialize(json& p_out_json) const override;

	void deserialize(const json& p_in_json) override;
};

enum class PathProcessError {
	EMPTY_PATH,
	INVALID_IDENTIFIER,
	UNDEFINED_WORKING_DIR,
};

enum class AssetLoadingError {
	FILE_ERROR,
	PARSING_ERROR,
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

	// Clear all asset registries and remove definitions.
	static void clear();

	static void clear_non_persistent();

	// Remove unusued assets from the registry
	static void collect_garbage();

	// Reload all loadable assets
	static void reload_all();

	/**
	 * Loads and registers the asset to the compatible asset registry.
	 * If given path already loaded creates a new handle pointing to that instance.
	 *
	 * @param p_prev_handle Custom asset handle to give. Used by scene deserialization.
	 */
	template <IsReflectedAsset T>
		requires IsLoadableAsset<T>
	static Result<AssetHandle, AssetLoadingError> load(
			const std::string& p_path, std::optional<AssetHandle> p_prev_handle = std::nullopt);

	// Creates and registers the asset to the compatible registry
	template <IsReflectedAsset T, typename... Args>
		requires IsCreatableAsset<T> || IsCreatableAsset<T, Args...>
	static std::optional<AssetHandle> create(Args&&... p_args);

	// Retrieve asset from registry
	template <IsReflectedAsset T> static std::shared_ptr<T> get(AssetHandle p_handle);

	template <IsReflectedAsset T> static std::shared_ptr<T> get_by_path(const std::string& p_path);

	// Retrieve asset metadata from registry
	template <IsReflectedAsset T>
	static std::optional<AssetMetadata> get_metadata(AssetHandle p_handle);

	/**
	 * Registers an asset type to the registry, type is guaranteed to get destroyed if no other
	 * reference exists.
	 *
	 * @param p_prev_handle Handle of the asset to be registered. Default is a random UID.
	 */
	template <IsReflectedAsset T>
	static AssetHandle register_asset(std::shared_ptr<T> p_asset, const std::string& p_path = "",
			std::optional<AssetHandle> p_prev_handle = std::nullopt);

	/**
	 * Registers an asset type to the registry that persists through garbage collection.
	 */
	template <IsReflectedAsset T>
	static AssetHandle register_asset_persistent(
			std::shared_ptr<T> p_asset, const std::string& p_path = "");

	// Release given asset handle from registry.
	template <IsReflectedAsset T> static bool free(AssetHandle p_handle);

	template <IsReflectedAsset T> static AssetRegistry<T>& get_registry();

	// Fetch all metadata objects of assets in the registry
	static std::unordered_map<AssetHandle, AssetMetadata> get_asset_metadata();

	// Transforms engine path format with suffix 'res://' to absolute path
	static Result<fs::path, PathProcessError> get_absolute_path(std::string_view p_path);

	static void serialize(json& p_json);
	static void deserialize(const json& p_json);

private:
	// type_name, registry map
	inline static std::unordered_map<std::string_view, IAssetRegistry*> s_registries;
};

} // namespace gl

#include "glitch/asset/asset_system.inl"
