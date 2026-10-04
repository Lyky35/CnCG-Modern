#pragma once
#include <vulkan/vulkan.h>

struct BlendState {
    bool enabled;
    VkBlendFactor srcColorFactor;
    VkBlendFactor dstColorFactor;
    VkBlendOp colorOp;
    VkBlendFactor srcAlphaFactor;
    VkBlendFactor dstAlphaFactor;
    VkBlendOp alphaOp;
};

struct DepthState {
    bool testEnabled;
    bool writeEnabled;
    VkCompareOp compareOp;
};

struct RasterizerState {
    VkCullModeFlags cullMode;
    VkPolygonMode fillMode;
    VkFrontFace frontFace;
    bool depthClamp;
    bool scissorEnable;
};

struct VulkanRenderState {
    VulkanRenderState();
    ~VulkanRenderState();

    void SetBlendState(const BlendState& state);
    void SetDepthState(const DepthState& state);
    void SetRasterizerState(const RasterizerState& state);

    const BlendState& GetBlendState() const { return m_blendState; }
    const DepthState& GetDepthState() const { return m_depthState; }
    const RasterizerState& GetRasterizerState() const { return m_rasterizerState; }

    VkPipelineColorBlendAttachmentState GetColorBlendAttachment() const;
    VkPipelineDepthStencilStateCreateInfo GetDepthStencilInfo() const;
    VkPipelineRasterizationStateCreateInfo GetRasterizerInfo() const;

private:
    BlendState m_blendState;
    DepthState m_depthState;
    RasterizerState m_rasterizerState;
};
