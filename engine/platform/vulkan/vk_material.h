#pragma once

#include "platform/vulkan/vk_common.h"

#include "platform/vulkan/vk_pipeline.h"

struct VulkanMaterialPipeline {
	VulkanPipeline pipeline;
	VulkanPipelineLayout pipeline_layout;
};

struct VulkanMaterialInstance {
    VulkanMaterialPipeline* pipeline;
    VkDescriptorSet* descriptor_set;
};
