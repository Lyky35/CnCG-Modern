#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanDescriptorManager {
public:
    VulkanDescriptorManager();
    ~VulkanDescriptorManager();

    bool Initialize(VkDevice device, uint32_t maxSets);
    void Shutdown();

    VkDescriptorSetLayout CreateSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    VkDescriptorPool CreatePool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes);
    VkDescriptorSet AllocateSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void UpdateSet(VkDescriptorSet set, const std::vector<VkWriteDescriptorSet>& writes);

private:
    VkDevice m_device;
};
