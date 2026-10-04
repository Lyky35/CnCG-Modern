#include "Vulkan2DRenderer.h"
#include "VulkanShader.h"
#include <cstring>
#include <cmath>

static const char* VERTEX_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;
void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    outTexCoord = inTexCoord;
    outColor = inColor;
}
)";

static const char* FRAGMENT_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D texSampler;
void main() {
    outColor = texture(texSampler, inTexCoord) * inColor;
}
)";

static const size_t MAX_BATCH_VERTICES = 65536;

static void ScreenToClip(float x, float y, int w, int h, float& cx, float& cy)
{
    cx = (x / static_cast<float>(w)) * 2.0f - 1.0f;
    cy = (y / static_cast<float>(h)) * 2.0f - 1.0f;
}

Vulkan2DRenderer::Vulkan2DRenderer()
    : m_renderer(nullptr)
    , m_pipeline(nullptr)
    , m_width(0)
    , m_height(0)
    , m_clipEnabled(false)
    , m_clipX(0)
    , m_clipY(0)
    , m_clipWidth(0)
    , m_clipHeight(0)
{
}

Vulkan2DRenderer::~Vulkan2DRenderer()
{
    Shutdown();
}

bool Vulkan2DRenderer::Initialize(VulkanRenderer* renderer, int width, int height)
{
    m_renderer = renderer;
    m_width = width;
    m_height = height;

    if (!CreateVertexBuffer())
        return false;

    if (!CreatePipeline())
        return false;

    return true;
}

void Vulkan2DRenderer::Shutdown()
{
    m_vertexBuffer.Destroy();
    if (m_pipeline)
    {
        m_pipeline->Destroy();
        delete m_pipeline;
        m_pipeline = nullptr;
    }
    m_renderer = nullptr;
    m_vertices.clear();
}

void Vulkan2DRenderer::Begin()
{
    m_vertices.clear();
}

void Vulkan2DRenderer::End()
{
    if (m_vertices.empty())
        return;

    VkCommandBuffer cmd = m_renderer->GetCurrentCommandBuffer();

    VkDeviceSize size = m_vertices.size() * sizeof(Vertex2D);
    void* data = m_vertexBuffer.Map();
    memcpy(data, m_vertices.data(), size);
    m_vertexBuffer.Unmap();

    m_renderer->SetPipeline(m_pipeline);

    if (m_clipEnabled)
        m_renderer->SetScissor(m_clipX, m_clipY, m_clipWidth, m_clipHeight);

    VkBuffer vertexBuffers[] = { m_vertexBuffer.GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(cmd, static_cast<uint32_t>(m_vertices.size()), 1, 0, 0);

    m_vertices.clear();
}

void Vulkan2DRenderer::DrawQuad(float x, float y, float width, float height, float r, float g, float b, float a)
{
    float x1, y1, x2, y2;
    ScreenToClip(x, y, m_width, m_height, x1, y1);
    ScreenToClip(x + width, y + height, m_width, m_height, x2, y2);

    Vertex2D v0 = { x1, y1, 0.0f, 0.0f, r, g, b, a };
    Vertex2D v1 = { x2, y1, 1.0f, 0.0f, r, g, b, a };
    Vertex2D v2 = { x2, y2, 1.0f, 1.0f, r, g, b, a };
    Vertex2D v3 = { x1, y2, 0.0f, 1.0f, r, g, b, a };

    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v0);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    if (m_vertices.size() >= MAX_BATCH_VERTICES)
        End();
}

void Vulkan2DRenderer::DrawTexturedQuad(float x, float y, float width, float height, VulkanTexture* texture, float r, float g, float b, float a)
{
    float x1, y1, x2, y2;
    ScreenToClip(x, y, m_width, m_height, x1, y1);
    ScreenToClip(x + width, y + height, m_width, m_height, x2, y2);

    Vertex2D v0 = { x1, y1, 0.0f, 0.0f, r, g, b, a };
    Vertex2D v1 = { x2, y1, 1.0f, 0.0f, r, g, b, a };
    Vertex2D v2 = { x2, y2, 1.0f, 1.0f, r, g, b, a };
    Vertex2D v3 = { x1, y2, 0.0f, 1.0f, r, g, b, a };

    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v0);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    if (m_vertices.size() >= MAX_BATCH_VERTICES)
        End();
}

void Vulkan2DRenderer::DrawLine(float x1, float y1, float x2, float y2, float width, float r, float g, float b, float a)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f)
        return;

    float nx = (-dy / len) * width * 0.5f;
    float ny = (dx / len) * width * 0.5f;

    float ax, ay, bx, by, cx, cy, dx2, dy2;
    ScreenToClip(x1 + nx, y1 + ny, m_width, m_height, ax, ay);
    ScreenToClip(x1 - nx, y1 - ny, m_width, m_height, bx, by);
    ScreenToClip(x2 - nx, y2 - ny, m_width, m_height, cx, cy);
    ScreenToClip(x2 + nx, y2 + ny, m_width, m_height, dx2, dy2);

    Vertex2D v0 = { ax, ay, 0.0f, 0.0f, r, g, b, a };
    Vertex2D v1 = { bx, by, 1.0f, 0.0f, r, g, b, a };
    Vertex2D v2 = { cx, cy, 1.0f, 1.0f, r, g, b, a };
    Vertex2D v3 = { dx2, dy2, 0.0f, 1.0f, r, g, b, a };

    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v0);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    if (m_vertices.size() >= MAX_BATCH_VERTICES)
        End();
}

void Vulkan2DRenderer::SetClipRect(int x, int y, int width, int height)
{
    m_clipEnabled = true;
    m_clipX = x;
    m_clipY = y;
    m_clipWidth = width;
    m_clipHeight = height;
}

void Vulkan2DRenderer::ClearClipRect()
{
    m_clipEnabled = false;
}

bool Vulkan2DRenderer::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = MAX_BATCH_VERTICES * sizeof(Vertex2D);
    return m_vertexBuffer.Create(
        m_renderer->GetDevice()->GetDevice(),
        m_renderer->GetDevice()->GetPhysicalDevice(),
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}

bool Vulkan2DRenderer::CreatePipeline()
{
    VkDevice device = m_renderer->GetDevice()->GetDevice();

    VulkanShader vertShader, fragShader;
    if (!vertShader.CreateFromSource(device, VERTEX_SHADER_SRC, VK_SHADER_STAGE_VERTEX_BIT))
        return false;
    if (!fragShader.CreateFromSource(device, FRAGMENT_SHADER_SRC, VK_SHADER_STAGE_FRAGMENT_BIT))
        return false;

    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerBinding;

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
    bindingDesc.stride = sizeof(Vertex2D);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescs[3] = {};
    attributeDescs[0].binding = 0;
    attributeDescs[0].location = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescs[0].offset = offsetof(Vertex2D, x);
    attributeDescs[1].binding = 0;
    attributeDescs[1].location = 1;
    attributeDescs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescs[1].offset = offsetof(Vertex2D, u);
    attributeDescs[2].binding = 0;
    attributeDescs[2].location = 2;
    attributeDescs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescs[2].offset = offsetof(Vertex2D, r);

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
    viewport.width = static_cast<float>(m_width);
    viewport.height = static_cast<float>(m_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };

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
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
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

    m_pipeline = new VulkanPipeline();
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    return true;
}
