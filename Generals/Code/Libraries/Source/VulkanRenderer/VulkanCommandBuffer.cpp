#include "VulkanCommandBuffer.h"

VulkanCommandBuffer::VulkanCommandBuffer()
    : m_device(VK_NULL_HANDLE)
    , m_pool(VK_NULL_HANDLE)
    , m_cmd(VK_NULL_HANDLE)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    Shutdown();
}

bool VulkanCommandBuffer::Initialize(VkDevice device, VkCommandPool pool)
{
    m_device = device;
    m_pool = pool;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &m_cmd) != VK_SUCCESS) {
        return false;
    }

    return true;
}

void VulkanCommandBuffer::Shutdown()
{
    if (m_device != VK_NULL_HANDLE && m_cmd != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(m_device, m_pool, 1, &m_cmd);
        m_cmd = VK_NULL_HANDLE;
    }
    m_device = VK_NULL_HANDLE;
    m_pool = VK_NULL_HANDLE;
}

void VulkanCommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_cmd, &beginInfo);
}

void VulkanCommandBuffer::End()
{
    vkEndCommandBuffer(m_cmd);
}

void VulkanCommandBuffer::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D extent, VkClearValue clearValue)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(m_cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(m_cmd);
}

void VulkanCommandBuffer::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint)
{
    vkCmdBindPipeline(m_cmd, bindPoint, pipeline);
}

void VulkanCommandBuffer::BindVertexBuffer(VkBuffer buffer, VkDeviceSize offset)
{
    vkCmdBindVertexBuffers(m_cmd, 0, 1, &buffer, &offset);
}

void VulkanCommandBuffer::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    vkCmdBindIndexBuffer(m_cmd, buffer, offset, indexType);
}

void VulkanCommandBuffer::BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set, VkPipelineBindPoint bindPoint)
{
    vkCmdBindDescriptorSets(m_cmd, bindPoint, layout, 0, 1, &set, 0, nullptr);
}

void VulkanCommandBuffer::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
    VkViewport viewport = {};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;

    vkCmdSetViewport(m_cmd, 0, 1, &viewport);
}

void VulkanCommandBuffer::SetScissor(int x, int y, int width, int height)
{
    VkRect2D scissor = {};
    scissor.offset = { x, y };
    scissor.extent = { (uint32_t)width, (uint32_t)height };

    vkCmdSetScissor(m_cmd, 0, 1, &scissor);
}

void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    vkCmdDrawIndexed(m_cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::DrawArrays(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(m_cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}
