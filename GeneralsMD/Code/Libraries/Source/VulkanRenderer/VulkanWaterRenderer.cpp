#include "VulkanWaterRenderer.h"
#include "VulkanShader.h"
#include <cstring>
#include <cmath>

static const char* WATER_VERTEX_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outWorldPos;
layout(binding = 0) uniform UniformBuffer {
    mat4 viewProjection;
    float time;
};
void main() {
    gl_Position = viewProjection * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
    outWorldPos = inPosition;
}
)";

static const char* WATER_FRAGMENT_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inWorldPos;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D waterTexture;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 0) uniform UniformBuffer {
    mat4 viewProjection;
    float time;
};
void main() {
    vec2 uv1 = inTexCoord + vec2(time * 0.02, time * 0.015);
    vec2 uv2 = inTexCoord * 1.5 + vec2(-time * 0.018, time * 0.022);
    vec3 normal1 = texture(normalMap, uv1).rgb * 2.0 - 1.0;
    vec3 normal2 = texture(normalMap, uv2).rgb * 2.0 - 1.0;
    vec3 combinedNormal = normalize(normal1 + normal2);
    vec3 viewDir = normalize(vec3(0.0, 1.0, 0.5));
    float fresnel = pow(1.0 - max(dot(viewDir, combinedNormal), 0.0), 3.0);
    vec4 waterColor = texture(waterTexture, inTexCoord + combinedNormal.xy * 0.05);
    vec3 deepColor = vec3(0.0, 0.15, 0.3);
    vec3 shallowColor = vec3(0.1, 0.4, 0.6);
    vec3 baseColor = mix(deepColor, shallowColor, fresnel);
    vec3 finalColor = mix(baseColor, waterColor.rgb, 0.6) + fresnel * 0.3;
    outColor = vec4(finalColor, 0.85);
}
)";

struct WaterVertex {
    float x, y, z;
    float u, v;
};

struct WaterUniformBuffer {
    Mat4 viewProjection;
    float time;
};

static const int WATER_GRID_SIZE = 64;
static const float WATER_PLANE_SIZE = 200.0f;

VulkanWaterRenderer::VulkanWaterRenderer()
    : m_renderer(nullptr)
    , m_waterTexture(nullptr)
    , m_normalMap(nullptr)
    , m_time(0.0f)
{
}

VulkanWaterRenderer::~VulkanWaterRenderer()
{
    Shutdown();
}

bool VulkanWaterRenderer::Initialize(VulkanRenderer* renderer)
{
    m_renderer = renderer;

    if (!CreateVertexBuffer())
        return false;

    if (!CreatePipeline())
        return false;

    return true;
}

void VulkanWaterRenderer::Shutdown()
{
    m_vertexBuffer.Destroy();
    m_renderer = nullptr;
    m_waterTexture = nullptr;
    m_normalMap = nullptr;
}

void VulkanWaterRenderer::SetWaterTexture(VulkanTexture* texture)
{
    m_waterTexture = texture;
}

void VulkanWaterRenderer::SetNormalMap(VulkanTexture* normalMap)
{
    m_normalMap = normalMap;
}

void VulkanWaterRenderer::SetTime(float time)
{
    m_time = time;
}

void VulkanWaterRenderer::Render(VulkanCamera* camera)
{
    if (!m_renderer || !m_waterTexture || !m_normalMap)
        return;

    VkCommandBuffer cmd = m_renderer->GetCurrentCommandBuffer();

    WaterUniformBuffer ubo = {};
    ubo.viewProjection = camera->GetViewProjection();
    ubo.time = m_time;

    VkBuffer vertexBuffers[] = { m_vertexBuffer.GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(cmd, WATER_GRID_SIZE * WATER_GRID_SIZE * 6, 1, 0, 0);
}

bool VulkanWaterRenderer::CreateVertexBuffer()
{
    int vertexCount = WATER_GRID_SIZE * WATER_GRID_SIZE * 6;
    VkDeviceSize bufferSize = vertexCount * sizeof(WaterVertex);

    if (!m_vertexBuffer.Create(
        m_renderer->GetDevice()->GetDevice(),
        m_renderer->GetDevice()->GetPhysicalDevice(),
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        return false;
    }

    WaterVertex* vertices = new WaterVertex[vertexCount];
    int index = 0;
    float halfSize = WATER_PLANE_SIZE * 0.5f;
    float step = WATER_PLANE_SIZE / WATER_GRID_SIZE;

    for (int z = 0; z < WATER_GRID_SIZE; z++)
    {
        for (int x = 0; x < WATER_GRID_SIZE; x++)
        {
            float x0 = -halfSize + x * step;
            float z0 = -halfSize + z * step;
            float x1 = x0 + step;
            float z1 = z0 + step;
            float u0 = static_cast<float>(x) / WATER_GRID_SIZE;
            float v0 = static_cast<float>(z) / WATER_GRID_SIZE;
            float u1 = static_cast<float>(x + 1) / WATER_GRID_SIZE;
            float v1 = static_cast<float>(z + 1) / WATER_GRID_SIZE;

            vertices[index++] = { x0, 0.0f, z0, u0, v0 };
            vertices[index++] = { x1, 0.0f, z0, u1, v0 };
            vertices[index++] = { x1, 0.0f, z1, u1, v1 };
            vertices[index++] = { x0, 0.0f, z0, u0, v0 };
            vertices[index++] = { x1, 0.0f, z1, u1, v1 };
            vertices[index++] = { x0, 0.0f, z1, u0, v1 };
        }
    }

    void* data = m_vertexBuffer.Map();
    memcpy(data, vertices, bufferSize);
    m_vertexBuffer.Unmap();
    delete[] vertices;

    return true;
}

bool VulkanWaterRenderer::CreatePipeline()
{
    VkDevice device = m_renderer->GetDevice()->GetDevice();

    VulkanShader vertShader, fragShader;
    if (!vertShader.CreateFromSource(device, WATER_VERTEX_SHADER_SRC, VK_SHADER_STAGE_VERTEX_BIT))
        return false;
    if (!fragShader.CreateFromSource(device, WATER_FRAGMENT_SHADER_SRC, VK_SHADER_STAGE_FRAGMENT_BIT))
        return false;

    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding waterTexBinding = {};
    waterTexBinding.binding = 1;
    waterTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    waterTexBinding.descriptorCount = 1;
    waterTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalMapBinding = {};
    normalMapBinding.binding = 2;
    normalMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalMapBinding.descriptorCount = 1;
    normalMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { uboBinding, waterTexBinding, normalMapBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        return false;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        return false;
    }

    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(WaterVertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescs[2] = {};
    attributeDescs[0].binding = 0;
    attributeDescs[0].location = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[0].offset = offsetof(WaterVertex, x);
    attributeDescs[1].binding = 0;
    attributeDescs[1].location = 1;
    attributeDescs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescs[1].offset = offsetof(WaterVertex, u);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    return true;
}
