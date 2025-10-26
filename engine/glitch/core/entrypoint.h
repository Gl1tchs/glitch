
/**
 * @file entrypoint.h
 */

#pragma once

#include "glitch/core/application.h"

extern gl::Application* create_application(gl::VectorView<const char*> p_args);

#ifdef GL_MAIN_IMPLEMENTATION
int main(int argc, const char** argv) {
	gl::VectorView<const char*> args(argv, argc);

	// create application instance
	gl::Application* _application = create_application(args);

	// start event loop
	_application->run();

	// cleanup
	delete _application;

	return 0;
}
#endif
