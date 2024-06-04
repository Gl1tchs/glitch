#include "renderer/renderer.h"

#include "renderer/node.h"

[[nodiscard]] GraphicsAPI find_proper_api() noexcept {
	return GRAPHICS_API_VULKAN;
}

static GraphicsAPI s_api;

Renderer::Renderer(Ref<Window> window) { s_api = find_proper_api(); }

Renderer::~Renderer() { _destroy_scene_graph(); }

void Renderer::wait_and_render() {}

SceneGraph& Renderer::get_scene_graph() { return scene_graph; }

RendererSettings& Renderer::get_settings() { return settings; }

RendererStats& Renderer::get_stats() { return stats; }

GraphicsAPI Renderer::get_graphics_api() { return s_api; }

void Renderer::_destroy_scene_graph() {
	scene_graph.traverse([](Node* node) {
		switch (node->get_type()) {
			case NodeType::NONE: {
				break;
			}
			case NodeType::GEOMETRY: {
#if 0
				Mesh* model = reinterpret_cast<Mesh*>(node);
				Mesh::destroy(model);
#endif
				break;
			}
			case NodeType::COMPUTE: {
#if 0
				ComputeEffectNode* compute_node =
						reinterpret_cast<ComputeEffectNode*>(node);
				ComputeEffectNode::destroy(compute_node);
#endif
				break;
			}
			case NodeType::CAMERA: {
				break;
			}
			case NodeType::LIGHT: {
				break;
			}
		}

		return false;
	});
}
