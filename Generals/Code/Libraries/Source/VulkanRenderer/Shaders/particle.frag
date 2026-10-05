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
