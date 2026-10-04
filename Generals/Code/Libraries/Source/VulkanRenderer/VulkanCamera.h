#pragma once
#include <vulkan/vulkan.h>
#include <cmath>

struct Mat4 {
    float m[4][4];

    static Mat4 Identity();
    static Mat4 Perspective(float fovY, float aspect, float zNear, float zFar);
    static Mat4 LookAt(float eye[3], float center[3], float up[3]);
    static Mat4 Multiply(const Mat4& a, const Mat4& b);
    static Mat4 Translate(float x, float y, float z);
    static Mat4 RotateY(float angle);
    static Mat4 Scale(float x, float y, float z);
};

class VulkanCamera {
public:
    VulkanCamera();
    ~VulkanCamera();

    void SetPerspective(float fovY, float aspect, float zNear, float zFar);
    void SetPosition(float x, float y, float z);
    void SetTarget(float x, float y, float z);
    void SetViewport(float x, float y, float width, float height);

    const Mat4& GetViewMatrix() const { return m_viewMatrix; }
    const Mat4& GetProjectionMatrix() const { return m_projMatrix; }
    const Mat4& GetViewProjection() const { return m_viewProj; }

    void UpdateViewProjection();

private:
    Mat4 m_viewMatrix;
    Mat4 m_projMatrix;
    Mat4 m_viewProj;
    float m_position[3];
    float m_target[3];
    float m_fovY;
    float m_aspect;
    float m_zNear;
    float m_zFar;
};
