#include "VulkanDescriptorManager.h"

VulkanDescriptorManager::VulkanDescriptorManager()
    : m_device(VK_NULL_HANDLE)
{
}

VulkanDescriptorManager::~VulkanDescriptorManager()
{
    Shutdown();
}

bool VulkanDescriptorManager::Initialize(VkDevice device, uint32_t maxSets)
{
    m_device = device;
    return true;
}

void VulkanDescriptorManager::Shutdown()
{
    m_device = VK_NULL_HANDLE;
}

VkDescriptorSetLayout VulkanDescriptorManager::CreateSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return layout;
}

VkDescriptorPool VulkanDescriptorManager::CreatePool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool pool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return pool;
}

VkDescriptorSet VulkanDescriptorManager::AllocateSet(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet set = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(m_device, &allocInfo, &set) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return set;
}

void VulkanDescriptorManager::UpdateSet(VkDescriptorSet set, const std::vector<VkWriteDescriptorSet>& writes)
{
    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
