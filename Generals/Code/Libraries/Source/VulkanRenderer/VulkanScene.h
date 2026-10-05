#pragma once
#include "VulkanCamera.h"
#include "VulkanRenderer.h"
#include <vector>

// Base for anything the scene can draw (terrain chunk, mesh, sprite, ...).
class VulkanRenderObject {
public:
    virtual ~VulkanRenderObject() {}
    virtual void Render(VulkanRenderer* renderer, VulkanCamera* camera) = 0;
};

class VulkanScene {
public:
    VulkanScene();
    ~VulkanScene();

    bool Initialize(VulkanRenderer* renderer);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void SetCamera(VulkanCamera* camera);
    VulkanCamera* GetCamera() { return m_camera; }

    void AddRenderObject(class VulkanRenderObject* obj);
    void RemoveRenderObject(class VulkanRenderObject* obj);

    void Render();

private:
    VulkanRenderer* m_renderer;
    VulkanCamera* m_camera;
    std::vector<class VulkanRenderObject*> m_objects;
};
