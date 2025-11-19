/**
 * @file asset_system.h
 *
 */

#pragma once

#include "glitch/asset/asset.h"
#include "glitch/core/templates/concepts.h"
#include "glitch/core/uid.h"

namespace gl {

using AssetHandle = UID;

struct AssetMetadata {
	const char* type_name;
	std::string path;
};

struct IAssetRegistry {
	virtual ~IAssetRegistry() = default;

	virtual size_t get_asset_size() const = 0;

	virtual void collect_garbage() = 0;

	virtual void clear() = 0;

	virtual std::unordered_map<AssetHandle, AssetMetadata> get_asset_metadatas() const = 0;

	virtual void serialize(json& p_out_json) const = 0;
	virtual void deserialize(const json& p_in_json) = 0;
};

template <typename T>
	requires IsReflectedAsset<T>
struct AssetRegistry : public IAssetRegistry {
	struct AssetEntry {
		std::shared_ptr<T> instance;
		std::string path;
	};

	std::unordered_map<AssetHandle, AssetEntry> assets;

	virtual ~AssetRegistry() = default;

	static AssetRegistry& get() {
		static AssetRegistry instance;
		return instance;
	}

	size_t get_asset_size() const override { return assets.size(); }

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

	std::unordered_map<AssetHandle, AssetMetadata> get_asset_metadatas() const override {
		std::unordered_map<AssetHandle, AssetMetadata> result;
		result.reserve(assets.size());

		for (auto& [handle, entry] : assets) {
			result.emplace(handle, AssetMetadata{ T::get_type_name(), entry.path });
		}

		return result;
	}

	void serialize(json& p_out_json) const override {
		// Only loadable assets can be (de)serialized
		if constexpr (IsLoadableAsset<T>) {
			json j;
			for (auto& [handle, entry] : assets) {
				// Do not serialize uninitialized assets
				if (!entry.instance) {
					continue;
				}

				// Only serialize non memory types
				if (!entry.path.empty() && !entry.path.starts_with("mem://")) {
					// Serialize metadata
					entry.instance->save(entry.path, entry.instance);

					j.push_back(json{
							{ "handle", handle },
							{ "path", entry.path },
					});
				}
			}

			if (j.size() > 0) {
				p_out_json = j;
			} else {
				p_out_json = json::value_t::null;
			}
		}
	}

	void deserialize(const json& p_in_json) override {
		// Only loadable assets can be (de)serialized
		if constexpr (IsLoadableAsset<T>) {
			if (!p_in_json.is_array()) {
				GL_LOG_ERROR("Unable to deserialize Assets of type '{}'.", T::get_type_name());
				return;
			}

			for (const auto& asset : p_in_json) {
				AssetHandle handle;
				AssetEntry entry;

				asset["handle"].get_to(handle);
				asset["path"].get_to(entry.path);

				assets[handle] = entry;
			}

			// Load all assets
			// TODO! do this in a seperate thread
			auto it = assets.begin();
			while (it != assets.end()) {
				// TODO! abs path
				// TODO! metadata
				if (const auto instance = T::load(it->second.path)) {
					it->second.instance = instance;

					it++;
				} else {
					it = assets.erase(it);
				}
			}
		}
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

	static void clear();

	/**
	 * Remove unusued assets from the registry
	 *
	 */
	static void collect_garbage();

	/**
	 * Loads and registers the asset to the compatible asset registry.
	 *
	 */
	template <typename T>
		requires IsLoadableAsset<T>
	static Result<AssetHandle, AssetLoadingError> load(std::string_view p_path) {
		const auto absolute_path = get_absolute_path(p_path);
		if (absolute_path.has_error()) {
			return make_err<AssetHandle>(AssetLoadingError::FILE_ERROR);
		}

		const std::shared_ptr<T> asset = T::load(*absolute_path);
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
		requires IsCreatableAsset<T> || IsCreatableAsset<T, Args...>
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

	static std::unordered_map<AssetHandle, AssetMetadata> get_asset_metadatas() {
		size_t total_asset_count = 0;
		for (const auto& [_, reg] : s_registries) {
			total_asset_count += reg->get_asset_size();
		}

		std::unordered_map<AssetHandle, AssetMetadata> result;
		result.reserve(total_asset_count);

		for (const auto& [_, reg] : s_registries) {
			auto metadatas = reg->get_asset_metadatas();
			result.merge(metadatas);
		}

		return result;
	}

	template <IsReflectedAsset T> static AssetRegistry<T>& get_registry() {
		auto& reg = AssetRegistry<T>::get();

		// Add asset registry to the asset system if not already existing
		constexpr const char* type_name = T::get_type_name();
		if (s_registries.find(type_name) == s_registries.end()) {
			s_registries[T::get_type_name()] = &reg;
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
	// type_name - registry map
	inline static std::unordered_map<std::string_view, IAssetRegistry*> s_registries;
};

} // namespace gl
