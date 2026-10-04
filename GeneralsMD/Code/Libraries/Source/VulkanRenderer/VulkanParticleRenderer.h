#pragma once
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanCamera.h"
#include <vulkan/vulkan.h>
#include <vector>

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float life;
    float maxLife;
    float size;
    float r, g, b, a;
};

class VulkanParticleRenderer {
public:
    VulkanParticleRenderer();
    ~VulkanParticleRenderer();

    bool Initialize(VulkanRenderer* renderer);
    void Shutdown();

    void SetTexture(VulkanTexture* texture);
    void EmitParticle(const Particle& particle);
    void Update(float deltaTime);
    void Render(VulkanCamera* camera);

private:
    bool CreateVertexBuffer();
    bool CreatePipeline();

    VulkanRenderer* m_renderer;
    VulkanBuffer m_vertexBuffer;
    VulkanTexture* m_texture;
    std::vector<Particle> m_particles;
    static const int MAX_PARTICLES = 10000;
};
