#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanShader {
public:
    VulkanShader();
    ~VulkanShader();

    bool Create(VkDevice device, const std::string& filename);
    bool CreateFromSource(VkDevice device, const std::string& source, VkShaderStageFlagBits stage);
    void Destroy();

    VkShaderModule GetModule() const { return m_module; }
    VkShaderStageFlagBits GetStage() const { return m_stage; }

private:
    VkDevice m_device;
    VkShaderModule m_module;
    VkShaderStageFlagBits m_stage;
};
