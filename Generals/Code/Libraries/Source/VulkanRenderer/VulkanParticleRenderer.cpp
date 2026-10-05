#include <string>
#include "VulkanParticleRenderer.h"
#include "VulkanShader.h"
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef VULKAN_GENERATED_SHADERS
#include "GeneratedShaders/particle_vert_spv.h"
static const std::string PARTICLE_VERTEX_SHADER_SRC((const char*)particle_vert_spv, particle_vert_spv_len);
#else
static const char* PARTICLE_VERTEX_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inSize;
layout(location = 2) in vec4 inColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(binding = 0) uniform UniformBuffer {
    mat4 viewProjection;
};
void main() {
    gl_Position = viewProjection * vec4(inPosition, 1.0);
    gl_PointSize = inSize;
    outColor = inColor;
    outTexCoord = vec2(0.0);
}
)";
#endif

#ifdef VULKAN_GENERATED_SHADERS
#include "GeneratedShaders/particle_frag_spv.h"
static const std::string PARTICLE_FRAGMENT_SHADER_SRC((const char*)particle_frag_spv, particle_frag_spv_len);
#else
static const char* PARTICLE_FRAGMENT_SHADER_SRC = R"(
#version 450
layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D particleTexture;
void main() {
    vec2 pc = gl_PointCoord * 2.0 - 1.0;
    float dist = dot(pc, pc);
    if (dist > 1.0)
        discard;
    float alpha = (1.0 - dist) * inColor.a;
    vec4 texColor = texture(particleTexture, gl_PointCoord);
    outColor = vec4(inColor.rgb * texColor.rgb, alpha);
}
)";
#endif

struct ParticleVertex {
    float x, y, z;
    float size;
    float r, g, b, a;
};

struct ParticleUniformBuffer {
    Mat4 viewProjection;
};

VulkanParticleRenderer::VulkanParticleRenderer()
    : m_renderer(nullptr)
    , m_texture(nullptr)
{
}

VulkanParticleRenderer::~VulkanParticleRenderer()
{
    Shutdown();
}

bool VulkanParticleRenderer::Initialize(VulkanRenderer* renderer)
{
    m_renderer = renderer;

    if (!CreateVertexBuffer())
        return false;

    if (!CreatePipeline())
        return false;

    return true;
}

void VulkanParticleRenderer::Shutdown()
{
    m_vertexBuffer.Destroy();
    m_renderer = nullptr;
    m_texture = nullptr;
    m_particles.clear();
}

void VulkanParticleRenderer::SetTexture(VulkanTexture* texture)
{
    m_texture = texture;
}

void VulkanParticleRenderer::EmitParticle(const Particle& particle)
{
    if (m_particles.size() >= MAX_PARTICLES)
        m_particles.erase(m_particles.begin());
    m_particles.push_back(particle);
}

void VulkanParticleRenderer::Update(float deltaTime)
{
    for (auto it = m_particles.begin(); it != m_particles.end();)
    {
        it->life -= deltaTime;
        if (it->life <= 0.0f)
        {
            it = m_particles.erase(it);
            continue;
        }
        it->x += it->vx * deltaTime;
        it->y += it->vy * deltaTime;
        it->z += it->vz * deltaTime;
        it->vy -= 9.8f * deltaTime;
        float lifeRatio = it->life / it->maxLife;
        it->a = lifeRatio;
        ++it;
    }
}

void VulkanParticleRenderer::Render(VulkanCamera* camera)
{
    if (!m_renderer || m_particles.empty())
        return;

    VkCommandBuffer cmd = m_renderer->GetCurrentCommandBuffer();

    size_t vertexCount = m_particles.size();
    VkDeviceSize bufferSize = vertexCount * sizeof(ParticleVertex);

    ParticleVertex* vertices = new ParticleVertex[vertexCount];
    for (size_t i = 0; i < vertexCount; i++)
    {
        const Particle& p = m_particles[i];
        vertices[i] = { p.x, p.y, p.z, p.size, p.r, p.g, p.b, p.a };
    }

    void* data = m_vertexBuffer.Map();
    memcpy(data, vertices, bufferSize);
    m_vertexBuffer.Unmap();
    delete[] vertices;

    ParticleUniformBuffer ubo = {};
    ubo.viewProjection = camera->GetViewProjection();

    VkBuffer vertexBuffers[] = { m_vertexBuffer.GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(cmd, static_cast<uint32_t>(vertexCount), 1, 0, 0);
}

bool VulkanParticleRenderer::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = MAX_PARTICLES * sizeof(ParticleVertex);
    return m_vertexBuffer.Create(
        m_renderer->GetDevice()->GetDevice(),
        m_renderer->GetDevice()->GetPhysicalDevice(),
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}

bool VulkanParticleRenderer::CreatePipeline()
{
    VkDevice device = m_renderer->GetDevice()->GetDevice();

    VulkanShader vertShader, fragShader;
    if (!vertShader.CreateFromSource(device, PARTICLE_VERTEX_SHADER_SRC, VK_SHADER_STAGE_VERTEX_BIT))
        return false;
    if (!fragShader.CreateFromSource(device, PARTICLE_FRAGMENT_SHADER_SRC, VK_SHADER_STAGE_FRAGMENT_BIT))
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

    VkDescriptorSetLayoutBinding bindings[] = { uboBinding, samplerBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
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
    bindingDesc.stride = sizeof(ParticleVertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescs[3] = {};
    attributeDescs[0].binding = 0;
    attributeDescs[0].location = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[0].offset = offsetof(ParticleVertex, x);
    attributeDescs[1].binding = 0;
    attributeDescs[1].location = 1;
    attributeDescs[1].format = VK_FORMAT_R32_SFLOAT;
    attributeDescs[1].offset = offsetof(ParticleVertex, size);
    attributeDescs[2].binding = 0;
    attributeDescs[2].location = 2;
    attributeDescs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescs[2].offset = offsetof(ParticleVertex, r);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
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
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
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
