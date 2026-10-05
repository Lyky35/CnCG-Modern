#include <string>
#include "VulkanTerrainRenderer.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include <cstring>
#include <cmath>
#include <vector>

struct TerrainUBO {
    Mat4 model;
    Mat4 view;
    Mat4 proj;
};

struct TerrainLightUBO {
    float lightDir[3];
    float lightColor[3];
    float ambientColor[3];
};

#ifdef VULKAN_GENERATED_SHADERS
#include "GeneratedShaders/terrain_vert_spv.h"
static const std::string TERRAIN_VERTEX_SHADER_SRC((const char*)terrain_vert_spv, terrain_vert_spv_len);
#else
static const char* TERRAIN_VERTEX_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outWorldPos;
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
    outNormal = mat3(ubo.model) * inNormal;
    outWorldPos = (ubo.model * vec4(inPosition, 1.0)).xyz;
}
)";
#endif

#ifdef VULKAN_GENERATED_SHADERS
#include "GeneratedShaders/terrain_frag_spv.h"
static const std::string TERRAIN_FRAGMENT_SHADER_SRC((const char*)terrain_frag_spv, terrain_frag_spv_len);
#else
static const char* TERRAIN_FRAGMENT_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inWorldPos;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D lightMapSampler;
layout(binding = 3) uniform LightUBO {
    vec3 lightDir;
    vec3 lightColor;
    vec3 ambientColor;
} light;
void main() {
    vec4 texColor = texture(texSampler, inTexCoord);
    vec4 lightColor = texture(lightMapSampler, inTexCoord);
    vec3 normal = normalize(inNormal);
    float diff = max(dot(normal, -lightDir), 0.0f);
    vec3 diffuse = diff * light.lightColor;
    vec3 ambient = light.ambientColor;
    vec3 lighting = ambient + diffuse;
    outColor = texColor * lightColor * vec4(lighting, 1.0);
}
)";
#endif

VulkanTerrainRenderer::VulkanTerrainRenderer()
    : m_renderer(nullptr)
    , m_texture(nullptr)
    , m_lightMap(nullptr)
    , m_indexCount(0)
    , m_width(0)
    , m_height(0)
    , m_scale(1.0f)
{
}

VulkanTerrainRenderer::~VulkanTerrainRenderer()
{
    Shutdown();
}

bool VulkanTerrainRenderer::Initialize(VulkanRenderer* renderer)
{
    m_renderer = renderer;
    return CreatePipeline();
}

void VulkanTerrainRenderer::Shutdown()
{
    m_vertexBuffer.Destroy();
    m_indexBuffer.Destroy();
    m_renderer = nullptr;
    m_texture = nullptr;
    m_lightMap = nullptr;
    m_indexCount = 0;
    m_width = 0;
    m_height = 0;
}

void VulkanTerrainRenderer::SetHeightMap(float* heights, int width, int height, float scale)
{
    m_width = width;
    m_height = height;
    m_scale = scale;

    std::vector<TerrainVertex> vertices;
    vertices.reserve(static_cast<size_t>(width) * height);

    for (int z = 0; z < height; z++)
    {
        for (int x = 0; x < width; x++)
        {
            float h = heights[z * width + x] * scale;

            float u = static_cast<float>(x) / static_cast<float>(width - 1);
            float v = static_cast<float>(z) / static_cast<float>(height - 1);

            float hL = (x > 0) ? heights[z * width + (x - 1)] * scale : h;
            float hR = (x < width - 1) ? heights[z * width + (x + 1)] * scale : h;
            float hD = (z > 0) ? heights[(z - 1) * width + x] * scale : h;
            float hU = (z < height - 1) ? heights[(z + 1) * width + x] * scale : h;

            float nx = hL - hR;
            float ny = 2.0f;
            float nz = hD - hU;
            float len = sqrtf(nx * nx + ny * ny + nz * nz);
            if (len > 0.0f)
            {
                nx /= len;
                ny /= len;
                nz /= len;
            }

            TerrainVertex vert = {};
            vert.x = static_cast<float>(x);
            vert.y = h;
            vert.z = static_cast<float>(z);
            vert.u = u;
            vert.v = v;
            vert.nx = nx;
            vert.ny = ny;
            vert.nz = nz;
            vertices.push_back(vert);
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(width - 1) * (height - 1) * 6);

    for (int z = 0; z < height - 1; z++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            uint32_t topLeft = static_cast<uint32_t>(z * width + x);
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = static_cast<uint32_t>((z + 1) * width + x);
            uint32_t bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    m_indexCount = static_cast<uint32_t>(indices.size());

    CreateVertexBuffer();
    CreateIndexBuffer();

    VkDevice device = m_renderer->GetDevice()->GetDevice();
    VkPhysicalDevice physicalDevice = m_renderer->GetDevice()->GetPhysicalDevice();

    VkDeviceSize vertexBufferSize = vertices.size() * sizeof(TerrainVertex);
    m_vertexBuffer.Create(device, physicalDevice, vertexBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* vertexData = m_vertexBuffer.Map();
    memcpy(vertexData, vertices.data(), static_cast<size_t>(vertexBufferSize));
    m_vertexBuffer.Unmap();

    VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);
    m_indexBuffer.Create(device, physicalDevice, indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* indexData = m_indexBuffer.Map();
    memcpy(indexData, indices.data(), static_cast<size_t>(indexBufferSize));
    m_indexBuffer.Unmap();
}

void VulkanTerrainRenderer::SetTexture(VulkanTexture* texture)
{
    m_texture = texture;
}

void VulkanTerrainRenderer::SetLightMap(VulkanTexture* lightMap)
{
    m_lightMap = lightMap;
}

void VulkanTerrainRenderer::Render(VulkanCamera* camera)
{
    if (m_indexCount == 0 || !m_renderer)
        return;

    VkCommandBuffer cmd = m_renderer->GetCurrentCommandBuffer();

    TerrainUBO ubo = {};
    ubo.model = Mat4::Identity();
    ubo.view = camera->GetViewMatrix();
    ubo.proj = camera->GetProjectionMatrix();

    TerrainLightUBO lightUbo = {};
    lightUbo.lightDir[0] = 0.0f;
    lightUbo.lightDir[1] = -1.0f;
    lightUbo.lightDir[2] = 0.0f;
    lightUbo.lightColor[0] = 1.0f;
    lightUbo.lightColor[1] = 1.0f;
    lightUbo.lightColor[2] = 1.0f;
    lightUbo.ambientColor[0] = 0.3f;
    lightUbo.ambientColor[1] = 0.3f;
    lightUbo.ambientColor[2] = 0.3f;

    VkBuffer vertexBuffers[] = { m_vertexBuffer.GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, m_indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(cmd, m_renderer->GetPipeline()->GetLayout(),
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TerrainUBO), &ubo);
    vkCmdPushConstants(cmd, m_renderer->GetPipeline()->GetLayout(),
        VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(TerrainUBO), sizeof(TerrainLightUBO), &lightUbo);

    vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, 0);
}

bool VulkanTerrainRenderer::CreateVertexBuffer()
{
    return true;
}

bool VulkanTerrainRenderer::CreateIndexBuffer()
{
    return true;
}

bool VulkanTerrainRenderer::CreatePipeline()
{
    VkDevice device = m_renderer->GetDevice()->GetDevice();

    VulkanShader vertShader, fragShader;
    if (!vertShader.CreateFromSource(device, TERRAIN_VERTEX_SHADER_SRC, VK_SHADER_STAGE_VERTEX_BIT))
        return false;
    if (!fragShader.CreateFromSource(device, TERRAIN_FRAGMENT_SHADER_SRC, VK_SHADER_STAGE_FRAGMENT_BIT))
        return false;

    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding lightMapBinding = {};
    lightMapBinding.binding = 2;
    lightMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lightMapBinding.descriptorCount = 1;
    lightMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding lightUboBinding = {};
    lightUboBinding.binding = 3;
    lightUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightUboBinding.descriptorCount = 1;
    lightUboBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboBinding, samplerBinding, lightMapBinding, lightUboBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        return false;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(TerrainUBO) + sizeof(TerrainLightUBO);
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        return false;
    }

    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(TerrainVertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescs[3] = {};
    attributeDescs[0].binding = 0;
    attributeDescs[0].location = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[0].offset = offsetof(TerrainVertex, x);
    attributeDescs[1].binding = 0;
    attributeDescs[1].location = 1;
    attributeDescs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescs[1].offset = offsetof(TerrainVertex, u);
    attributeDescs[2].binding = 0;
    attributeDescs[2].location = 2;
    attributeDescs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[2].offset = offsetof(TerrainVertex, nx);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 1280.0f;
    viewport.height = 720.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { 1280, 720 };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShader.GetModule();
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShader.GetModule();
    shaderStages[1].pName = "main";

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = m_renderer->GetRenderPass();
    pipelineInfo.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        return false;
    }

    VulkanPipeline* terrainPipeline = new VulkanPipeline();
    terrainPipeline->Create(device, m_renderer->GetRenderPass(), 1280, 720);
    m_renderer->SetPipeline(terrainPipeline);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    return true;
}
