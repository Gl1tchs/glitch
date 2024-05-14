#include "core/entrypoint.h"

int main(int argc, const char** argv) {
	// create application instance
	Application* _application = create_application(argc, argv);

	// start event loop
	_application->run();

	// cleanup
	delete _application;

	return 0;
}
