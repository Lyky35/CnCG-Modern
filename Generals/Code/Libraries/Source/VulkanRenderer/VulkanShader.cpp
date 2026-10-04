#include "VulkanShader.h"
#include <fstream>
#include <vector>
#include <stdexcept>

VulkanShader::VulkanShader()
    : m_device(VK_NULL_HANDLE)
    , m_module(VK_NULL_HANDLE)
    , m_stage(VK_SHADER_STAGE_VERTEX_BIT)
{
}

VulkanShader::~VulkanShader()
{
    Destroy();
}

bool VulkanShader::Create(VkDevice device, const std::string& filename)
{
    m_device = device;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        return false;

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &m_module) != VK_SUCCESS)
        return false;

    return true;
}

bool VulkanShader::CreateFromSource(VkDevice device, const std::string& source, VkShaderStageFlagBits stage)
{
    m_device = device;
    m_stage = stage;

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = source.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(source.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &m_module) != VK_SUCCESS)
        return false;

    return true;
}

void VulkanShader::Destroy()
{
    if (m_module != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_device, m_module, nullptr);
        m_module = VK_NULL_HANDLE;
    }
}
