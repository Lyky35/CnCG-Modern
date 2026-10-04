#pragma once
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCamera.h"
#include <vulkan/vulkan.h>

class VulkanShadowRenderer {
public:
    VulkanShadowRenderer();
    ~VulkanShadowRenderer();

    bool Initialize(VulkanRenderer* renderer, int shadowMapSize);
    void Shutdown();

    void BeginShadowPass();
    void EndShadowPass();

    void SetLightDirection(float x, float y, float z);
    void SetLightViewProjection(const Mat4& vp);

    VulkanTexture* GetShadowMap() { return &m_shadowMap; }
    const Mat4& GetLightVP() const { return m_lightVP; }

private:
    bool CreateShadowMap();
    bool CreateRenderPass();
    bool CreateFramebuffer();
    bool CreatePipeline();

    VulkanRenderer* m_renderer;
    VulkanTexture m_shadowMap;
    VulkanBuffer m_uniformBuffer;
    VkRenderPass m_renderPass;
    VkFramebuffer m_framebuffer;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;
    int m_shadowMapSize;
    Mat4 m_lightVP;
    float m_lightDir[3];
    VkCommandBuffer m_commandBuffer;
    VkDescriptorSet m_descriptorSet;
    VkDescriptorPool m_descriptorPool;
};
