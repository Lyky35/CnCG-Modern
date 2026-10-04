#pragma once
#include <vulkan/vulkan.h>

class VulkanTexture {
public:
    VulkanTexture();
    ~VulkanTexture();

    bool Create(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
    void Destroy();

    VkImage GetImage() const { return m_image; }
    VkImageView GetImageView() const { return m_imageView; }
    VkSampler GetSampler() const { return m_sampler; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    VkFormat GetFormat() const { return m_format; }

    void TransitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);

private:
    VkDevice m_device;
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkImageView m_imageView;
    VkSampler m_sampler;
    uint32_t m_width;
    uint32_t m_height;
    VkFormat m_format;
};
