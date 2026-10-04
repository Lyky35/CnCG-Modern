#include "VulkanCamera.h"
#include <cstring>

Mat4 Mat4::Identity() {
    Mat4 result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.m[i][j] = (i == j) ? 1.0f : 0.0f;
    return result;
}

Mat4 Mat4::Perspective(float fovY, float aspect, float zNear, float zFar) {
    Mat4 result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.m[i][j] = 0.0f;

    float tanHalfFovY = tanf(fovY * 0.5f);

    result.m[0][0] = 1.0f / (aspect * tanHalfFovY);
    result.m[1][1] = 1.0f / tanHalfFovY;
    result.m[2][2] = zFar / (zNear - zFar);
    result.m[2][3] = -1.0f;
    result.m[3][2] = (zNear * zFar) / (zNear - zFar);

    return result;
}

Mat4 Mat4::LookAt(float eye[3], float center[3], float up[3]) {
    float f[3] = {
        center[0] - eye[0],
        center[1] - eye[1],
        center[2] - eye[2]
    };
    float fLen = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    f[0] /= fLen;
    f[1] /= fLen;
    f[2] /= fLen;

    float s[3] = {
        f[1] * up[2] - f[2] * up[1],
        f[2] * up[0] - f[0] * up[2],
        f[0] * up[1] - f[1] * up[0]
    };
    float sLen = sqrtf(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    s[0] /= sLen;
    s[1] /= sLen;
    s[2] /= sLen;

    float u[3] = {
        s[1] * f[2] - s[2] * f[1],
        s[2] * f[0] - s[0] * f[2],
        s[0] * f[1] - s[1] * f[0]
    };

    Mat4 result = Mat4::Identity();
    result.m[0][0] = s[0];
    result.m[0][1] = s[1];
    result.m[0][2] = s[2];
    result.m[1][0] = u[0];
    result.m[1][1] = u[1];
    result.m[1][2] = u[2];
    result.m[2][0] = -f[0];
    result.m[2][1] = -f[1];
    result.m[2][2] = -f[2];
    result.m[3][0] = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
    result.m[3][1] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
    result.m[3][2] = f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2];

    return result;
}

Mat4 Mat4::Multiply(const Mat4& a, const Mat4& b) {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return result;
}

Mat4 Mat4::Translate(float x, float y, float z) {
    Mat4 result = Mat4::Identity();
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
}

Mat4 Mat4::RotateY(float angle) {
    Mat4 result = Mat4::Identity();
    float c = cosf(angle);
    float s = sinf(angle);
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    return result;
}

Mat4 Mat4::Scale(float x, float y, float z) {
    Mat4 result = Mat4::Identity();
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    return result;
}

VulkanCamera::VulkanCamera()
    : m_fovY(60.0f * 3.14159265f / 180.0f)
    , m_aspect(16.0f / 9.0f)
    , m_zNear(0.1f)
    , m_zFar(1000.0f)
{
    m_position[0] = 0.0f;
    m_position[1] = 0.0f;
    m_position[2] = 0.0f;
    m_target[0] = 0.0f;
    m_target[1] = 0.0f;
    m_target[2] = 0.0f;

    m_viewMatrix = Mat4::Identity();
    m_projMatrix = Mat4::Perspective(m_fovY, m_aspect, m_zNear, m_zFar);
    m_viewProj = Mat4::Identity();
}

VulkanCamera::~VulkanCamera() {
}

void VulkanCamera::SetPerspective(float fovY, float aspect, float zNear, float zFar) {
    m_fovY = fovY;
    m_aspect = aspect;
    m_zNear = zNear;
    m_zFar = zFar;
    m_projMatrix = Mat4::Perspective(fovY, aspect, zNear, zFar);
    UpdateViewProjection();
}

void VulkanCamera::SetPosition(float x, float y, float z) {
    m_position[0] = x;
    m_position[1] = y;
    m_position[2] = z;
    UpdateViewProjection();
}

void VulkanCamera::SetTarget(float x, float y, float z) {
    m_target[0] = x;
    m_target[1] = y;
    m_target[2] = z;
    UpdateViewProjection();
}

void VulkanCamera::SetViewport(float x, float y, float width, float height) {
}

void VulkanCamera::UpdateViewProjection() {
    float up[3] = { 0.0f, 1.0f, 0.0f };
    m_viewMatrix = Mat4::LookAt(m_position, m_target, up);
    m_viewProj = Mat4::Multiply(m_projMatrix, m_viewMatrix);
}
