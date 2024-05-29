#include "gl/renderer/renderer.h"

#include "gl/renderer/compute.h"
#include "gl/renderer/model.h"
#include "gl/renderer/node.h"

#include "platform/vulkan/vk_renderer.h"

[[nodiscard]] RenderBackend find_proper_backend() noexcept {
	return RenderBackend::Vulkan;
}

static RenderBackend s_backend;

SceneGraph& Renderer::get_scene_graph() { return scene_graph; }

RendererSettings& Renderer::get_settings() { return settings; }

RenderBackend Renderer::get_backend() { return s_backend; }

Ref<Renderer> Renderer::create(RenderBackend backend, Ref<Window> window) {
	s_backend = backend;

	switch (backend) {
		case RenderBackend::Vulkan: {
			// create and initialize vulkan renderer
			auto context = create_ref<VulkanRenderer>(window);

			return context;
		}
		default: {
			return nullptr;
		}
	}
}

void Renderer::_destroy_scene_graph() {
	scene_graph.traverse([](Node* node) {
		switch (node->get_type()) {
			case NodeType::NONE: {
				break;
			}
			case NodeType::GEOMETRY: {
				Model* model = reinterpret_cast<Model*>(node);
				Model::destroy(model);
				break;
			}
			case NodeType::COMPUTE: {
				ComputeEffectNode* compute_node =
						reinterpret_cast<ComputeEffectNode*>(node);
				ComputeEffectNode::destroy(compute_node);
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
