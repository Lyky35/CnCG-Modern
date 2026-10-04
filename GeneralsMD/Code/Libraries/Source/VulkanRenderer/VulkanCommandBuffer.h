#pragma once
#include <vulkan/vulkan.h>

class VulkanCommandBuffer {
public:
    VulkanCommandBuffer();
    ~VulkanCommandBuffer();

    bool Initialize(VkDevice device, VkCommandPool pool);
    void Shutdown();

    void Begin();
    void End();

    void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, VkClearValue clearValue);
    void EndRenderPass();

    void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint);
    void BindVertexBuffer(VkBuffer buffer, VkDeviceSize offset);
    void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
    void BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set, VkPipelineBindPoint bindPoint);

    void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
    void SetScissor(int x, int y, int width, int height);

    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void DrawArrays(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

    VkCommandBuffer GetHandle() const { return m_cmd; }

private:
    VkDevice m_device;
    VkCommandPool m_pool;
    VkCommandBuffer m_cmd;
};
