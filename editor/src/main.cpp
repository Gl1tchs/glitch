#define GL_MAIN_IMPLEMENTATION
#include <glitch/core/entrypoint.h>

#include "editor.h"

Application* create_application(gl::VectorView<const char*> p_args) {
	ApplicationCreateInfo info = {
		.name = "Glitch Editor",
		.args = p_args,
	};

	Application* app = new Application(info);
	app->push_layer<EditorLayer>();

	return app;
}
