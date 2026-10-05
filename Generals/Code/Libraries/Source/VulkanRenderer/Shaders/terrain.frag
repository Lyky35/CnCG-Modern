#version 450
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inWorldPos;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D lightMapSampler;
layout(binding = 3) uniform LightUBO {
    vec3 lightDir;
    vec3 lightColor;
    vec3 ambientColor;
};
void main() {
    vec4 texColor = texture(texSampler, inTexCoord);
    vec4 lightMapColor = texture(lightMapSampler, inTexCoord);
    vec3 normal = normalize(inNormal);
    float diff = max(dot(normal, -lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    vec3 ambient = ambientColor;
    vec3 lighting = ambient + diffuse;
    outColor = texColor * lightMapColor * vec4(lighting, 1.0);
}
