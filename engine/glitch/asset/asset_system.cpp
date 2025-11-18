#include "glitch/asset/asset_system.h"

#include "glitch/platform/os.h"

namespace gl {

void AssetSystem::init() { shutdown(); }

void AssetSystem::shutdown() {
	for (auto& reg : s_registries) {
		reg->clear();
		reg->reset();
	}

	s_registries.clear();
}

void AssetSystem::collect_garbage() {
	for (auto& reg : s_registries) {
		reg->collect_garbage();
	}
}

/**
 * Transforms engine path format with suffix 'res://' to absolute path
 *
 */
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
	for (const auto& reg : s_registries) {
		reg->serialize(p_json);
	}
}

void AssetSystem::deserialize(const json& p_json) {}

} //namespace gl
