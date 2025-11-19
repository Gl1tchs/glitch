/**
 * @file script.h
 *
 */

#pragma once

#include "glitch/scripting/script_engine.h"

namespace gl {

struct Script {
	std::string script_path;

	ScriptRef script = 0;
	bool is_loaded = false;

	// metadata cache for reloading
	std::optional<ScriptMetadata> metadata = std::nullopt;

	ScriptResult load();

	void unload();

	void reset();
};

// TODO metadata
GL_DEFINE_SERIALIZABLE(Script, script_path, metadata);

} //namespace gl