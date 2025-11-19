#pragma once

#include "glitch/asset/asset_system.h"

namespace gl {

template <IsReflectedAsset T> AssetRegistry<T>& AssetRegistry<T>::get() {
	static AssetRegistry instance;
	return instance;
}

template <IsReflectedAsset T> size_t AssetRegistry<T>::get_asset_size() const {
	return assets.size();
}

template <IsReflectedAsset T> void AssetRegistry<T>::collect_garbage() {
	std::erase_if(assets, [](const auto& item) {
		// use_count 1 means only the map holds it
		return item.second.instance.use_count() == 1;
	});
}

template <IsReflectedAsset T>
AssetHandle AssetRegistry<T>::register_asset(
		std::shared_ptr<T> p_asset, const std::string& p_path) {
	AssetHandle handle; // new random uid
	assets.insert_or_assign(handle, AssetEntry{ p_asset, p_path });
	return handle;
}

template <IsReflectedAsset T> std::shared_ptr<T> AssetRegistry<T>::get_asset(AssetHandle p_handle) {
	const auto it = assets.find(p_handle);
	if (it == assets.end()) {
		return nullptr;
	}

	return it->second.instance;
}

template <IsReflectedAsset T>
std::optional<AssetMetadata> AssetRegistry<T>::get_metadata(AssetHandle p_handle) {
	const auto it = assets.find(p_handle);
	if (it == assets.end()) {
		return std::nullopt;
	}

	return AssetMetadata{ T::get_type_name(), it->second.path };
}

template <IsReflectedAsset T> bool AssetRegistry<T>::erase(AssetHandle p_handle) {
	return assets.erase(p_handle) > 0;
}

template <IsReflectedAsset T> void AssetRegistry<T>::clear() { assets.clear(); }

template <IsReflectedAsset T>
std::unordered_map<AssetHandle, AssetMetadata> AssetRegistry<T>::get_asset_metadata() const {
	std::unordered_map<AssetHandle, AssetMetadata> result;
	result.reserve(assets.size());

	for (auto& [handle, entry] : assets) {
		result.emplace(handle, AssetMetadata{ T::get_type_name(), entry.path });
	}

	return result;
}

template <IsReflectedAsset T> void AssetRegistry<T>::serialize(json& p_out_json) const {
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

template <IsReflectedAsset T> void AssetRegistry<T>::deserialize(const json& p_in_json) {
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
		auto it = assets.begin();
		while (it != assets.end()) {
			const auto path = AssetSystem::get_absolute_path(it->second.path);
			if (!path) {
				it++;
				continue;
			}

			if (const auto instance = T::load(path.get_value())) {
				it->second.instance = instance;

				it++;
			} else {
				it = assets.erase(it);
			}
		}
	}
}

template <IsReflectedAsset T>
	requires IsLoadableAsset<T>
Result<AssetHandle, AssetLoadingError> AssetSystem::load(std::string_view p_path) {
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

template <IsReflectedAsset T, typename... Args>
	requires IsCreatableAsset<T> || IsCreatableAsset<T, Args...>
std::optional<AssetHandle> AssetSystem::create(Args&&... p_args) {
	const std::shared_ptr<T> asset = T::create(std::forward<Args>(p_args)...);
	if (!asset) {
		return std::nullopt;
	}

	auto& registry = get_registry<T>();
	return registry.register_asset(asset, "");
}

template <IsReflectedAsset T> std::shared_ptr<T> AssetSystem::get(AssetHandle p_handle) {
	auto& registry = get_registry<T>();
	return registry.get_asset(p_handle);
}

template <IsReflectedAsset T>
std::optional<AssetMetadata> AssetSystem::get_metadata(AssetHandle p_handle) {
	auto& registry = get_registry<T>();
	return registry.get_metadata(p_handle);
}

template <IsReflectedAsset T>
AssetHandle AssetSystem::register_asset(std::shared_ptr<T> p_asset, const std::string& p_path) {
	auto& registry = get_registry<T>();
	return registry.register_asset(p_asset, p_path);
}

template <IsReflectedAsset T> bool AssetSystem::free(AssetHandle p_handle) {
	auto& registry = get_registry<T>();
	return registry.erase(p_handle);
}

template <IsReflectedAsset T> AssetRegistry<T>& AssetSystem::get_registry() {
	auto& reg = AssetRegistry<T>::get();

	// Add asset registry to the asset system if not already existing
	constexpr const char* type_name = T::get_type_name();
	if (s_registries.find(type_name) == s_registries.end()) {
		s_registries[T::get_type_name()] = &reg;
	}

	return reg;
}

}; //namespace gl