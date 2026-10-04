#include "VulkanScene.h"

VulkanScene::VulkanScene()
    : m_renderer(nullptr)
    , m_camera(nullptr)
{
}

VulkanScene::~VulkanScene() {
    Shutdown();
}

bool VulkanScene::Initialize(VulkanRenderer* renderer) {
    m_renderer = renderer;
    return true;
}

void VulkanScene::Shutdown() {
    m_objects.clear();
    m_camera = nullptr;
    m_renderer = nullptr;
}

void VulkanScene::BeginFrame() {
    if (m_renderer) {
        m_renderer->BeginFrame();
    }
}

void VulkanScene::EndFrame() {
    if (m_renderer) {
        m_renderer->EndFrame();
    }
}

void VulkanScene::SetCamera(VulkanCamera* camera) {
    m_camera = camera;
}

void VulkanScene::AddRenderObject(VulkanRenderObject* obj) {
    if (obj) {
        m_objects.push_back(obj);
    }
}

void VulkanScene::RemoveRenderObject(VulkanRenderObject* obj) {
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (*it == obj) {
            m_objects.erase(it);
            return;
        }
    }
}

void VulkanScene::Render() {
    if (!m_renderer) {
        return;
    }

    for (auto* obj : m_objects) {
        if (obj) {
            obj->Render(m_renderer, m_camera);
        }
    }
}
