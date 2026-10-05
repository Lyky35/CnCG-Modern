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
