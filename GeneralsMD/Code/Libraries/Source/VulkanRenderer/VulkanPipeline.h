#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanPipeline {
public:
    VulkanPipeline();
    ~VulkanPipeline();

    bool Create(VkDevice device, VkRenderPass renderPass, uint32_t width, uint32_t height);
    void Destroy();

    VkPipeline GetPipeline() const { return m_pipeline; }
    VkPipelineLayout GetLayout() const { return m_layout; }

    void SetBlendState(bool enable, VkBlendFactor srcFactor, VkBlendFactor dstFactor);
    void SetDepthState(bool enable, bool writeEnable, VkCompareOp compareOp);
    void SetRasterizerState(VkCullModeFlags cullMode, VkPolygonMode fillMode);

private:
    VkDevice m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
};
