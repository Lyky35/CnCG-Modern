#version 450
layout(binding = 0) uniform LightUBO {
    mat4 viewProjection;
};
layout(location = 0) in vec3 inPosition;
void main() {
    gl_Position = viewProjection * vec4(inPosition, 1.0);
}
