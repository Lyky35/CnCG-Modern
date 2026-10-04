#pragma once
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include <vulkan/vulkan.h>
#include <vector>

struct Vertex2D {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class Vulkan2DRenderer {
public:
    Vulkan2DRenderer();
    ~Vulkan2DRenderer();

    bool Initialize(VulkanRenderer* renderer, int width, int height);
    void Shutdown();

    void Begin();
    void End();

    void DrawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
    void DrawTexturedQuad(float x, float y, float width, float height, VulkanTexture* texture, float r, float g, float b, float a);
    void DrawLine(float x1, float y1, float x2, float y2, float width, float r, float g, float b, float a);

    void SetClipRect(int x, int y, int width, int height);
    void ClearClipRect();

private:
    bool CreateVertexBuffer();
    bool CreatePipeline();

    VulkanRenderer* m_renderer;
    VulkanBuffer m_vertexBuffer;
    VulkanPipeline* m_pipeline;
    std::vector<Vertex2D> m_vertices;
    int m_width;
    int m_height;
    bool m_clipEnabled;
    int m_clipX, m_clipY, m_clipWidth, m_clipHeight;
};
