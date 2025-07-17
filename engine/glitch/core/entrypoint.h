
/**
 * @file entrypoint.h
 */

#pragma once

#include "glitch/core/application.h"

extern gl::Application* create_application(int p_argc, const char** p_argv);

#ifdef GL_MAIN_IMPLEMENTATION
int main(int argc, const char** argv) {
	// create application instance
	gl::Application* _application = create_application(argc, argv);

	// start event loop
	_application->run();

	// cleanup
	delete _application;

	return 0;
}
#endif
