#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanDevice {
public:
    VulkanDevice();
    ~VulkanDevice();

    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void BeginFrame();
    void EndFrame();
    void Present();

    VkDevice GetDevice() const { return m_device; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
    VkCommandPool GetCommandPool() const { return m_commandPool; }
    VkSwapchainKHR GetSwapchain() const { return m_swapchain; }
    uint32_t GetGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
    uint32_t GetSwapchainImageCount() const { return m_swapchainImageCount; }
    VkExtent2D GetSwapchainExtent() const { return m_swapchainExtent; }
    VkFormat GetSwapchainFormat() const { return m_swapchainFormat; }

    VkCommandBuffer AllocateCommandBuffer();
    void FreeCommandBuffer(VkCommandBuffer cmd);

    void RecreateSwapchain(int width, int height);

private:
    bool CreateInstance();
    bool SelectPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSwapchain(int width, int height);
    bool CreateCommandPool();
    bool CreateSyncObjects();

    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    uint32_t m_graphicsQueueFamily;
    VkCommandPool m_commandPool;
    VkSwapchainKHR m_swapchain;
    VkExtent2D m_swapchainExtent;
    VkFormat m_swapchainFormat;
    uint32_t m_swapchainImageCount;

    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    static const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame;

    bool m_validationLayersEnabled;
};
