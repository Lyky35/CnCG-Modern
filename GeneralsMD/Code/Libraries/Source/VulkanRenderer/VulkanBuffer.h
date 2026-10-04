#pragma once
#include <vulkan/vulkan.h>

class VulkanBuffer {
public:
    VulkanBuffer();
    ~VulkanBuffer();

    bool Create(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void Destroy();

    void* Map();
    void Unmap();

    void CopyFrom(const void* data, VkDeviceSize size);

    VkBuffer GetBuffer() const { return m_buffer; }
    VkDeviceMemory GetMemory() const { return m_memory; }
    VkDeviceSize GetSize() const { return m_size; }

    static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    VkDevice m_device;
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    VkDeviceSize m_size;
};
