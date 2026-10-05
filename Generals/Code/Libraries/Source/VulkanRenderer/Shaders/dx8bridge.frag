#version 450
// DX8-on-Vulkan bridge fragment shader.
// Two texture stages, common fixed-function combiners, optional alpha test.

layout(location = 0) in vec4  vColor;
layout(location = 1) in vec2  vUV0;
layout(location = 2) in vec2  vUV1;
layout(location = 3) flat in uint vDiffuseRaw;
layout(location = 4) flat in uint vSpecRaw;

layout(binding = 0) uniform texture2D tex[2];
layout(binding = 2) uniform sampler     samp;

layout(set = 0, binding = 1, std140) uniform DrawUBO {
    mat4 mvp;
    vec4 params;
    vec4 viewport;
    vec4 material;
    vec4 light;
    vec4 lightDir;
    vec4 ambient;
    vec4 alphaRef;
    vec4 stage;       // x: stage0 op, y: stage1 op, z: stage0 uv index, w: stage1 uv index
} u;

layout(location = 0) out vec4 outColor;

// stage op ids (normalized on the CPU from D3DTOP_*)
const int OP_PASS    = 0;  // DISABLE / SELECTARG1(current)
const int OP_MOD     = 1;  // MODULATE
const int OP_REPLACE = 2;  // SELECTARG2/DECAL: texture replaces
const int OP_ADD     = 3;  // ADD / ADDSIGNED / ADDSMOOTH
const int OP_BLEND   = 4;  // BLENDCURRENTALPHA / BLENDTEXTUREALPHA approximations

vec4 unpackColor(uint c)
{
    return vec4(float( c        & 0xffu),
                float((c >>  8) & 0xffu),
                float((c >> 16) & 0xffu),
                float((c >> 24) & 0xffu)) / 255.0;
}

vec4 applyStage(vec4 cur, int op, int texIdx, int uvIdx)
{
    if (op == OP_PASS) return cur;
    vec2 uv = uvIdx == 0 ? vUV0 : vUV1;
    vec4 t = texture(sampler2D(tex[texIdx], samp), uv);
    if (op == OP_MOD)     return cur * t;
    if (op == OP_REPLACE) return t;
    if (op == OP_ADD)     return cur + t;
    if (op == OP_BLEND)   return vec4(mix(cur.rgb, t.rgb, t.a), cur.a);
    return cur * t;
}

void main()
{
    vec4 color = vColor;

    int op0 = int(u.stage.x);
    if (op0 != OP_PASS)
        color = applyStage(color, op0, 0, int(u.stage.z));

    int op1 = int(u.stage.y);
    if (op1 != OP_PASS)
        color = applyStage(color, op1, 1, int(u.stage.w));

    // D3DTA_SELECTARG2 on stage0 with no texture falls back to diffuse/specular
    if (op0 == -1) color = unpackColor(vDiffuseRaw);

    if (u.alphaRef.x > 0.5 && color.a * 255.0 < u.alphaRef.y * 255.0 - 0.5)
        discard;

    outColor = color;
}
