#include "glitch/scripting/script.h"

namespace gl {

ScriptResult Script::load() {
	const Result<ScriptRef, ScriptResult> res = ScriptEngine::load_script_file(script_path);
	if (res.has_error()) {
		is_loaded = false;
		return res.get_error();
	}

	script = res.get_value();
	is_loaded = true;

	if (!metadata) {
		// Reset metadata
		metadata = ScriptEngine::get_metadata(script);
	} else {
		// Set metadata values to lua
		ScriptEngine::set_metadata(script, *metadata);
	}

	return ScriptResult::SUCCESS;
}

void Script::unload() {
	// TODO: destroy the script somehow, somewhere
	ScriptEngine::unload_script(script);
	script = 0;
	metadata = std::nullopt;
	is_loaded = false;
}

void Script::reset() {
	if (metadata) {
		for (const auto& [name, field] : metadata->fields) {
			if (!ScriptEngine::set_field(script, name.c_str(), field)) {
				GL_LOG_ERROR("[Script::reset] Unable to set metadata field for script: {}\nKey "
							 "{} do not exists.",
						script_path, name);
			}
		}
	}
}

} // namespace gl