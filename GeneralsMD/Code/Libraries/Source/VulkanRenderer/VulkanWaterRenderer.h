#pragma once
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCamera.h"
#include <vulkan/vulkan.h>

class VulkanWaterRenderer {
public:
    VulkanWaterRenderer();
    ~VulkanWaterRenderer();

    bool Initialize(VulkanRenderer* renderer);
    void Shutdown();

    void SetWaterTexture(VulkanTexture* texture);
    void SetNormalMap(VulkanTexture* normalMap);
    void SetTime(float time);

    void Render(VulkanCamera* camera);

private:
    bool CreateVertexBuffer();
    bool CreatePipeline();

    VulkanRenderer* m_renderer;
    VulkanBuffer m_vertexBuffer;
    VulkanTexture* m_waterTexture;
    VulkanTexture* m_normalMap;
    float m_time;
};
