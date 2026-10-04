#pragma once
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void BeginFrame();
    void EndFrame();
    void Present();

    void Clear(bool clearColor, bool clearDepth, float r, float g, float b, float a);
    void SetViewport(float x, float y, float width, float height);
    void SetScissor(int x, int y, int width, int height);

    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void DrawArrays(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

    void SetTexture(uint32_t slot, VulkanTexture* texture);
    void SetPipeline(VulkanPipeline* pipeline);

    VulkanDevice* GetDevice() { return &m_device; }
    VkRenderPass GetRenderPass() const { return m_renderPass; }
    VkCommandBuffer GetCurrentCommandBuffer() const { return m_currentCmd; }

    void RecreateSwapchain(int width, int height);

private:
    VulkanDevice m_device;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandBuffer m_currentCmd;

    bool CreateRenderPass();
    bool CreateFramebuffers();
};
