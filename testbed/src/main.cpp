#define GL_MAIN_IMPLEMENTATION
#include <glitch/core/entrypoint.h>

#include "game.h"

Application* create_application(int argc, const char** argv) {
	ApplicationCreateInfo info = {
		.name = "Glitch Sandbox",
		.argc = argc,
		.argv = argv,
	};
	return new Game(info);
}
