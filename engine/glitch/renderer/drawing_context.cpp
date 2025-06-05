#include "glitch/renderer/drawing_context.h"

void DrawingContext::assign_scene_graph(SceneGraph p_graph) {
	// Update the transforms
	p_graph.update_transforms();

	// TODO: filter and eleminate unwanted elements
	root = p_graph.get_root();
}
