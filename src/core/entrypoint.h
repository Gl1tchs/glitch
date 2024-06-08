#pragma once

#include "core/application.h"

extern Application* create_application(int p_argc, const char** p_argv);

#ifdef GL_MAIN_IMPLEMENTATION
int main(int argc, const char** argv) {
	// create application instance
	Application* _application = create_application(argc, argv);

	// start event loop
	_application->run();

	// cleanup
	delete _application;

	return 0;
}
#endif
