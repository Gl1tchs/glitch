#define GL_MAIN_IMPLEMENTATION
#include <glitch/core/entrypoint.h>

#include "editor.h"

Application* create_application(int argc, const char** argv) {
	ApplicationCreateInfo info = {
		.name = "Glitch Cockpit",
		.argc = argc,
		.argv = argv,
	};
	return new EditorApplication(info);
}
