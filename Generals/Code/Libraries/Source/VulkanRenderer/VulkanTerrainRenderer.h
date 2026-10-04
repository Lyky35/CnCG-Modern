#pragma once
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCamera.h"
#include <vulkan/vulkan.h>
#include <vector>

struct TerrainVertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
};

class VulkanTerrainRenderer {
public:
    VulkanTerrainRenderer();
    ~VulkanTerrainRenderer();

    bool Initialize(VulkanRenderer* renderer);
    void Shutdown();

    void SetHeightMap(float* heights, int width, int height, float scale);
    void SetTexture(VulkanTexture* texture);
    void SetLightMap(VulkanTexture* lightMap);

    void Render(VulkanCamera* camera);

private:
    bool CreateVertexBuffer();
    bool CreateIndexBuffer();
    bool CreatePipeline();

    VulkanRenderer* m_renderer;
    VulkanBuffer m_vertexBuffer;
    VulkanBuffer m_indexBuffer;
    VulkanTexture* m_texture;
    VulkanTexture* m_lightMap;
    uint32_t m_indexCount;
    int m_width;
    int m_height;
    float m_scale;
};
