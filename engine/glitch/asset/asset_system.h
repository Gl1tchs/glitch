/**
 * @file asset_system.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/core/uid.h"

namespace gl {

using AssetHandle = UID;

struct IAssetRegistry {
	virtual ~IAssetRegistry() = default;

	virtual void collect_garbage() = 0;

	virtual void clear() = 0;

	virtual void reset() = 0;

	virtual void serialize(json& p_json) const = 0;
	virtual void deserialize(const json& p_json) = 0;
};

template <IsReflectedAsset T> struct AssetRegistry : public IAssetRegistry {
	struct AssetEntry {
		std::shared_ptr<T> instance;
		std::string path;
	};

	std::unordered_map<AssetHandle, AssetEntry> assets;
	bool is_registered = false;

	virtual ~AssetRegistry() = default;

	static AssetRegistry& get() {
		static AssetRegistry instance;
		return instance;
	}

	void collect_garbage() override {
		std::erase_if(assets, [](const auto& item) {
			// use_count 1 means only the map holds it
			return item.second.instance.use_count() == 1;
		});
	}

	/**
	 * Registers given asset to the registry, so that it would automatically deleted
	 * if no other reference is pointing to it.
	 *
	 * @param p_path Path of the asset to be registered
	 */
	AssetHandle register_asset(std::shared_ptr<T> p_asset, const std::string& p_path) {
		AssetHandle handle; // new random uid
		assets.insert_or_assign(handle, AssetEntry{ p_asset, p_path });
		return handle;
	}

	std::shared_ptr<T> get_asset(AssetHandle p_handle) {
		const auto it = assets.find(p_handle);
		if (it == assets.end()) {
			return nullptr;
		}

		return it->second.instance;
	}

	bool erase(AssetHandle p_handle) { return assets.erase(p_handle) > 0; }

	void clear() override { assets.clear(); }

	void reset() override { is_registered = false; }

	void serialize(json& p_out_json) const override {
		json list = json::array();
		for (auto& [handle, entry] : assets) {
			if (!entry.path.empty()) {
				list.push_back({ { "uid", handle }, { "path", entry.path } });
			}
		}

		p_out_json[T::get_type_name()] = list;
	}

	void deserialize(const json& p_in_json) override {
		// TODO!
	}
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
	template <typename T, typename... Args>
		requires IsLoadableAsset<T, Args...>
	static Result<AssetHandle, AssetLoadingError> load(std::string_view p_path, Args&&... p_args) {
		const auto absolute_path = get_absolute_path(p_path);
		if (absolute_path.has_error()) {
			return make_err<AssetHandle>(AssetLoadingError::FILE_ERROR);
		}

		const std::shared_ptr<T> asset = T::load(*absolute_path, std::forward<Args>(p_args)...);
		if (!asset) {
			return make_err<AssetHandle>(AssetLoadingError::PARSING_ERROR);
		}

		auto& registry = get_registry<T>();
		return registry.register_asset(asset, absolute_path.get_value().string());
	}

	/**
	 * Creates and registers the asset to the compatible registry
	 *
	 */
	template <typename T, typename... Args>
		requires IsCreatableAsset<T, Args...>
	static std::optional<AssetHandle> create(Args&&... p_args) {
		const std::shared_ptr<T> asset = T::create(std::forward<Args>(p_args)...);
		if (!asset) {
			return std::nullopt;
		}

		auto& registry = get_registry<T>();
		return registry.register_asset(asset, "");
	}

	/**
	 * Retrieve asset from registry
	 *
	 */
	template <typename T> static std::shared_ptr<T> get(AssetHandle p_handle) {
		auto& registry = get_registry<T>();
		return registry.get_asset(p_handle);
	}

	template <typename T>
	static AssetHandle register_asset(std::shared_ptr<T> p_asset, const std::string& p_path = "") {
		auto& registry = get_registry<T>();
		return registry.register_asset(p_asset, p_path);
	}

	/**
	 * Release given asset handle from registry.
	 *
	 */
	template <typename T> static bool free(AssetHandle p_handle) {
		auto& registry = get_registry<T>();
		return registry.erase(p_handle);
	}

	template <typename T> static AssetRegistry<T>& get_registry() {
		auto& reg = AssetRegistry<T>::get();

		// Add asset registry to the asset system
		if (!reg.is_registered) {
			s_registries.push_back(&reg);
			reg.is_registered = true;
		}

		return reg;
	}

	/**
	 * Transforms engine path format with suffix 'res://' to absolute path
	 *
	 */
	static Result<fs::path, PathProcessError> get_absolute_path(std::string_view p_path);

	static void serialize(json& p_json);
	static void deserialize(const json& p_json);

private:
	inline static std::vector<IAssetRegistry*> s_registries;
};

} // namespace gl
