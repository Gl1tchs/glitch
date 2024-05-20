#define GL_MAIN
#include "core/entrypoint.h"

Application* create_application(int argc, const char** argv) {
	ApplicationCreateInfo info = {
		.name = "GlEditor",
		.argc = argc,
		.argv = argv,
	};
	return new Application(info);
}
