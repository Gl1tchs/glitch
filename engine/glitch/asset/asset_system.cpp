#include "glitch/asset/asset_system.h"

#include "glitch/platform/os.h"

namespace gl {

void AssetSystem::clear() {
	for (auto& [_, reg] : s_registries) {
		reg->clear();
	}

	s_registries.clear();
}

void AssetSystem::collect_garbage() {
	for (auto& [_, reg] : s_registries) {
		reg->collect_garbage();
	}
}

std::unordered_map<AssetHandle, AssetMetadata> AssetSystem::get_asset_metadata() {
	size_t total_asset_count = 0;
	for (const auto& [_, reg] : s_registries) {
		total_asset_count += reg->get_asset_size();
	}

	std::unordered_map<AssetHandle, AssetMetadata> result;
	result.reserve(total_asset_count);

	for (const auto& [_, reg] : s_registries) {
		auto metadatas = reg->get_asset_metadata();
		result.merge(metadatas);
	}

	return result;
}

Result<fs::path, PathProcessError> AssetSystem::get_absolute_path(std::string_view p_path) {
	if (p_path.empty()) {
		return make_err<fs::path>(PathProcessError::EMPTY_PATH);
	}

	if (!p_path.starts_with("res://")) {
		if (p_path.find("://") == std::string::npos) {
			return fs::path(p_path.begin(), p_path.end());
		} else {
			// TODO: this is wrong
			return make_err<fs::path>(PathProcessError::INVALID_IDENTIFIER);
		}
	}

	// TODO! make this the project directory
	const char* working_dir = os::getenv("GL_WORKING_DIR");
	if (!working_dir) {
		return make_err<fs::path>(PathProcessError::UNDEFINED_WORKING_DIR);
	}

	constexpr size_t identifier_len = sizeof("ref://") - 1; // minus \0

	fs::path absolute_path = working_dir;
	absolute_path /= p_path.substr(identifier_len);

	return absolute_path;
}

void AssetSystem::serialize(json& p_json) {
	p_json = json();
	for (const auto& [type_name, reg] : s_registries) {
		json j;
		reg->serialize(j);

		if (!j.is_null()) {
			p_json[type_name] = j;
		}
	}
}

void AssetSystem::deserialize(const json& p_json) {
	// Clear the assets
	// NOTE: We do not call clear() here to be able to access registry keys
	for (auto& [_, reg] : s_registries) {
		reg->clear();
	}

	for (const auto& [type_name, items] : p_json.items()) {
		const auto it = s_registries.find(type_name);
		if (it != s_registries.end()) {
			IAssetRegistry* reg = it->second;
			reg->deserialize(items);
		} else {
			GL_LOG_WARNING("[AssetSystem::deserialize] Asset type '{}' found in save file but not "
						   "registered in AssetSystem.",
					type_name);
		}
	}
}

} //namespace gl
