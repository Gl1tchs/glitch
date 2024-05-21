#define GL_MAIN
#include <gl/core/entrypoint.h>

#include "testbed.h"

Application* create_application(int argc, const char** argv) {
	ApplicationCreateInfo info = {
		.name = "GlEditor",
		.argc = argc,
		.argv = argv,
	};
	return new TestBedApplication(info);
}
