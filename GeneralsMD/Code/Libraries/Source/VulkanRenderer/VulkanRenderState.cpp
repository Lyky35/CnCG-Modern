#include "VulkanRenderState.h"

VulkanRenderState::VulkanRenderState()
{
    m_blendState = {};
    m_blendState.enabled = false;
    m_blendState.srcColorFactor = VK_BLEND_FACTOR_ONE;
    m_blendState.dstColorFactor = VK_BLEND_FACTOR_ZERO;
    m_blendState.colorOp = VK_BLEND_OP_ADD;
    m_blendState.srcAlphaFactor = VK_BLEND_FACTOR_ONE;
    m_blendState.dstAlphaFactor = VK_BLEND_FACTOR_ZERO;
    m_blendState.alphaOp = VK_BLEND_OP_ADD;

    m_depthState = {};
    m_depthState.testEnabled = true;
    m_depthState.writeEnabled = true;
    m_depthState.compareOp = VK_COMPARE_OP_LESS;

    m_rasterizerState = {};
    m_rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
    m_rasterizerState.fillMode = VK_POLYGON_MODE_FILL;
    m_rasterizerState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    m_rasterizerState.depthClamp = false;
    m_rasterizerState.scissorEnable = false;
}

VulkanRenderState::~VulkanRenderState()
{
}

void VulkanRenderState::SetBlendState(const BlendState& state)
{
    m_blendState = state;
}

void VulkanRenderState::SetDepthState(const DepthState& state)
{
    m_depthState = state;
}

void VulkanRenderState::SetRasterizerState(const RasterizerState& state)
{
    m_rasterizerState = state;
}

VkPipelineColorBlendAttachmentState VulkanRenderState::GetColorBlendAttachment() const
{
    VkPipelineColorBlendAttachmentState attachment = {};
    attachment.blendEnable = m_blendState.enabled ? VK_TRUE : VK_FALSE;
    attachment.srcColorBlendFactor = m_blendState.srcColorFactor;
    attachment.dstColorBlendFactor = m_blendState.dstColorFactor;
    attachment.colorBlendOp = m_blendState.colorOp;
    attachment.srcAlphaBlendFactor = m_blendState.srcAlphaFactor;
    attachment.dstAlphaBlendFactor = m_blendState.dstAlphaFactor;
    attachment.alphaBlendOp = m_blendState.alphaOp;
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    return attachment;
}

VkPipelineDepthStencilStateCreateInfo VulkanRenderState::GetDepthStencilInfo() const
{
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = m_depthState.testEnabled ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = m_depthState.writeEnabled ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = m_depthState.compareOp;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    return depthStencil;
}

VkPipelineRasterizationStateCreateInfo VulkanRenderState::GetRasterizerInfo() const
{
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = m_rasterizerState.depthClamp ? VK_TRUE : VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = m_rasterizerState.fillMode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = m_rasterizerState.cullMode;
    rasterizer.frontFace = m_rasterizerState.frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;
    return rasterizer;
}
