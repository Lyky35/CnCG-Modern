#version 450
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inWorldPos;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D waterTexture;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 0) uniform UniformBuffer {
    mat4 viewProjection;
    float time;
};
void main() {
    vec2 uv1 = inTexCoord + vec2(time * 0.02, time * 0.015);
    vec2 uv2 = inTexCoord * 1.5 + vec2(-time * 0.018, time * 0.022);
    vec3 normal1 = texture(normalMap, uv1).rgb * 2.0 - 1.0;
    vec3 normal2 = texture(normalMap, uv2).rgb * 2.0 - 1.0;
    vec3 combinedNormal = normalize(normal1 + normal2);
    vec3 viewDir = normalize(vec3(0.0, 1.0, 0.5));
    float fresnel = pow(1.0 - max(dot(viewDir, combinedNormal), 0.0), 3.0);
    vec4 waterColor = texture(waterTexture, inTexCoord + combinedNormal.xy * 0.05);
    vec3 deepColor = vec3(0.0, 0.15, 0.3);
    vec3 shallowColor = vec3(0.1, 0.4, 0.6);
    vec3 baseColor = mix(deepColor, shallowColor, fresnel);
    vec3 finalColor = mix(baseColor, waterColor.rgb, 0.6) + fresnel * 0.3;
    outColor = vec4(finalColor, 0.85);
}
