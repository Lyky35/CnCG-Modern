#version 450
// DX8-on-Vulkan bridge vertex shader.
// Every D3D8 FVF is re-staged on the CPU into this canonical attribute layout.

layout(location = 0) in vec4  inPos;       // x,y,z,w (w=1; XYZRHW: x,y pixels, w=1/rhw)
layout(location = 1) in uint  inDiffuse;   // D3DCOLOR 0xAARRGGBB
layout(location = 2) in uint  inSpecular;  // D3DCOLOR 0xAARRGGBB
layout(location = 3) in vec2  inUV0;
layout(location = 4) in vec2  inUV1;
layout(location = 5) in vec4  inMisc;      // xyz = normal

layout(set = 0, binding = 1, std140) uniform DrawUBO {
    mat4 mvp;         // world * view * proj (identity when RHW passthrough)
    vec4 params;      // x: RHW flag, y: lighting enabled
    vec4 viewport;    // x,y: render target size in pixels
    vec4 material;    // rgb: diffuse material, a: opacity
    vec4 light;       // rgb: diffuse light color, a: enabled
    vec4 lightDir;
    vec4 ambient;     // rgb: ambient (material*light ambient folded)
    vec4 alphaRef;    // x: alpha-test enabled, y: reference/255
    vec4 stage;       // x: stage0 op, y: stage1 op, z: stage0 uv index, w: stage1 uv index
} u;

layout(location = 0) out vec4  vColor;
layout(location = 1) out vec2  vUV0;
layout(location = 2) out vec2  vUV1;
layout(location = 3) flat out uint vDiffuseRaw;
layout(location = 4) flat out uint vSpecRaw;

vec4 unpackColor(uint c)
{
    return vec4(float( c        & 0xffu),
                float((c >>  8) & 0xffu),
                float((c >> 16) & 0xffu),
                float((c >> 24) & 0xffu)) / 255.0;
}

void main()
{
    vUV0 = inUV0;
    vUV1 = inUV1;
    vDiffuseRaw = inDiffuse;
    vSpecRaw    = inSpecular;

    vec4 color = unpackColor(inDiffuse);

    if (u.params.x > 0.5) {
        float invW = inPos.w == 0.0 ? 1.0 : inPos.w;
        float x = inPos.x / u.viewport.x * 2.0 - 1.0;
        float y = 1.0 - inPos.y / u.viewport.y * 2.0;
        gl_Position = vec4(x * invW, y * invW, inPos.z, invW);
    } else {
        gl_Position = u.mvp * vec4(inPos.xyz, 1.0);
        if (u.params.y > 0.5 && u.light.w > 0.5) {
            vec3 n = normalize(inMisc.xyz);
            float diff = max(dot(n, -normalize(u.lightDir.xyz)), 0.0);
            color.rgb = color.rgb * u.material.rgb * (u.ambient.rgb + u.light.rgb * diff);
        } else {
            color.rgb = color.rgb * u.material.rgb;
        }
        color.a *= u.material.a;
    }
    vColor = color;
}
