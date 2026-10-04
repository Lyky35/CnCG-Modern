#include "VulkanRenderer.h"
#include <stdexcept>

VulkanRenderer::VulkanRenderer()
    : m_renderPass(VK_NULL_HANDLE)
    , m_currentCmd(VK_NULL_HANDLE)
{
}

VulkanRenderer::~VulkanRenderer()
{
    Shutdown();
}

bool VulkanRenderer::Initialize(HWND hwnd, int width, int height)
{
    if (!m_device.Initialize(hwnd, width, height)) {
        return false;
    }

    if (!CreateRenderPass()) {
        return false;
    }

    if (!CreateFramebuffers()) {
        return false;
    }

    return true;
}

void VulkanRenderer::Shutdown()
{
    if (m_device.GetDevice()) {
        vkDeviceWaitIdle(m_device.GetDevice());
    }

    for (auto& framebuffer : m_framebuffers) {
        if (framebuffer) vkDestroyFramebuffer(m_device.GetDevice(), framebuffer, nullptr);
    }
    m_framebuffers.clear();

    if (m_renderPass) {
        vkDestroyRenderPass(m_device.GetDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    m_device.Shutdown();
}

bool VulkanRenderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_device.GetSwapchainFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device.GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        return false;
    }

    return true;
}

bool VulkanRenderer::CreateFramebuffers()
{
    uint32_t imageCount = m_device.GetSwapchainImageCount();
    m_framebuffers.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageView attachments[] = { m_device.m_swapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_device.GetSwapchainExtent().width;
        framebufferInfo.height = m_device.GetSwapchainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device.GetDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

void VulkanRenderer::BeginFrame()
{
    m_currentCmd = m_device.AllocateCommandBuffer();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_currentCmd, &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[0];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_device.GetSwapchainExtent();

    VkClearValue clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_currentCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::EndFrame()
{
    vkCmdEndRenderPass(m_currentCmd);
    vkEndCommandBuffer(m_currentCmd);
}

void VulkanRenderer::Present()
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_currentCmd;

    vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device.GetGraphicsQueue());

    m_device.FreeCommandBuffer(m_currentCmd);
    m_currentCmd = VK_NULL_HANDLE;
}

void VulkanRenderer::Clear(bool clearColor, bool clearDepth, float r, float g, float b, float a)
{
    if (!m_currentCmd) return;

    VkClearAttachment attachments[2] = {};
    uint32_t attachmentCount = 0;

    if (clearColor) {
        attachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        attachments[0].colorAttachment = 0;
        attachments[0].clearValue.color = { { r, g, b, a } };
        attachmentCount++;
    }

    if (clearDepth) {
        attachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        attachments[1].clearValue.depthStencil = { 1.0f, 0 };
        attachmentCount++;
    }

    if (attachmentCount > 0) {
        VkClearRect rect = {};
        rect.rect.offset = { 0, 0 };
        rect.rect.extent = m_device.GetSwapchainExtent();
        rect.baseArrayLayer = 0;
        rect.layerCount = 1;

        vkCmdClearAttachments(m_currentCmd, attachmentCount, attachments, 1, &rect);
    }
}

void VulkanRenderer::SetViewport(float x, float y, float width, float height)
{
    if (!m_currentCmd) return;

    VkViewport viewport = {};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_currentCmd, 0, 1, &viewport);
}

void VulkanRenderer::SetScissor(int x, int y, int width, int height)
{
    if (!m_currentCmd) return;

    VkRect2D scissor = {};
    scissor.offset = { x, y };
    scissor.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    vkCmdSetScissor(m_currentCmd, 0, 1, &scissor);
}

void VulkanRenderer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    if (!m_currentCmd) return;

    vkCmdDrawIndexed(m_currentCmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRenderer::DrawArrays(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    if (!m_currentCmd) return;

    vkCmdDraw(m_currentCmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderer::SetTexture(uint32_t slot, VulkanTexture* texture)
{
    (void)slot;
    (void)texture;
}

void VulkanRenderer::SetPipeline(VulkanPipeline* pipeline)
{
    if (!m_currentCmd || !pipeline) return;

    vkCmdBindPipeline(m_currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
}

void VulkanRenderer::RecreateSwapchain(int width, int height)
{
    m_device.RecreateSwapchain(width, height);

    for (auto& framebuffer : m_framebuffers) {
        if (framebuffer) vkDestroyFramebuffer(m_device.GetDevice(), framebuffer, nullptr);
    }
    m_framebuffers.clear();

    CreateFramebuffers();
}
