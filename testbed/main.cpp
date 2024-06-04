#define GL_MAIN_IMPLEMENTATION
#include <core/entrypoint.h>

#include "testbed.h"

Application* create_application(int argc, const char** argv) {
	ApplicationCreateInfo info = {
		.name = "Glitch Testbed",
		.argc = argc,
		.argv = argv,
	};
	return new TestBedApplication(info);
}
