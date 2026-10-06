/*
**	DX8Vk bridge implementation: Vulkan-backed IDirect3D8/IDirect3DDevice8 COM
**	surface for WW3D2. See dx8vk.h for scope notes.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3d8.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "dx8vk.h"
#include "VulkanDevice.h"
#include "GeneratedShaders/dx8bridge_vert_spv.h"
#include "GeneratedShaders/dx8bridge_frag_spv.h"


static void dx8vk_log(const char* fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    OutputDebugStringA("[dx8vk] ");
    OutputDebugStringA(buf);
    static FILE* lf = NULL;
    if (!lf) lf = fopen("dx8vk.log", "a");
    if (lf) { fprintf(lf, "[dx8vk] %s\n", buf); fflush(lf); }
}

static void dx8vk_log_once(const char* tag, const char* fmt, ...)
{
    static std::vector<std::string> seen;
    for (size_t i = 0; i < seen.size(); i++)
        if (seen[i] == tag) return;
    seen.push_back(tag);
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    dx8vk_log("%s", buf);
}

/* ------------------------------------------------------------------ */
/* format mapping                                                       */
/* ------------------------------------------------------------------ */

static VkFormat vk_format_for(D3DFORMAT f)
{
    switch ((int)f) {
        case D3DFMT_A8R8G8B8:  return VK_FORMAT_B8G8R8A8_UNORM;
        case D3DFMT_X8R8G8B8:  return VK_FORMAT_B8G8R8A8_UNORM;
        case D3DFMT_R8G8B8:    return VK_FORMAT_R8G8B8_UNORM;
        case D3DFMT_R5G6B5:    return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case D3DFMT_A1R5G5B5:  return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        case D3DFMT_A4R4G4B4:  return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case D3DFMT_A8:        return VK_FORMAT_R8_UNORM;
        case D3DFMT_L8:        return VK_FORMAT_R8_UNORM;
        case D3DFMT_A8L8:      return VK_FORMAT_R8G8_UNORM;
        case D3DFMT_V8U8:      return VK_FORMAT_R8G8_SNORM;
        case D3DFMT_DXT1:      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case D3DFMT_DXT2:
        case D3DFMT_DXT3:      return VK_FORMAT_BC2_UNORM_BLOCK;
        case D3DFMT_DXT4:
        case D3DFMT_DXT5:      return VK_FORMAT_BC3_UNORM_BLOCK;
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D16:       return VK_FORMAT_D16_UNORM;
        case D3DFMT_D24X8:
        case D3DFMT_D24X4S4:
        case D3DFMT_D24S8:     return VK_FORMAT_D24_UNORM_S8_UINT;
        case D3DFMT_D32:            return VK_FORMAT_D32_SFLOAT;
        default:               return VK_FORMAT_UNDEFINED;
    }
}

static int d3dfmt_bpp(D3DFORMAT f)
{
    switch ((int)f) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_A2B10G10R10:
 return 4;
        case D3DFMT_R5G6B5:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4: return 2;
        case D3DFMT_R8G8B8: return 3;
        case D3DFMT_A8:
        case D3DFMT_L8: return 1;
        case D3DFMT_A8L8:
        case D3DFMT_V8U8: return 2;
        case D3DFMT_DXT1: return 1;   // block-bytes per texel: 8/16
        case D3DFMT_DXT3:
        case D3DFMT_DXT5: return 1;
        case D3DFMT_D16:
        case D3DFMT_D24S8: return 4;
        default: return 4;
    }
}

static bool d3dfmt_is_compressed(D3DFORMAT f)
{
    return (int)f == D3DFMT_DXT1 || (int)f == D3DFMT_DXT2 || (int)f == D3DFMT_DXT3
        || (int)f == D3DFMT_DXT4 || (int)f == D3DFMT_DXT5;
}

static UINT fvf_size(DWORD fvf)
{
    UINT size = 0;
    switch (fvf & D3DFVF_POSITION_MASK) {
        case D3DFVF_XYZ:            size += 12; break;
        case D3DFVF_XYZRHW:         size += 16; break;
        case D3DFVF_XYZB1:          size += 16; break;
        case D3DFVF_XYZB2:          size += 20; break;
        case D3DFVF_XYZB3:          size += 24; break;
        case D3DFVF_XYZB4:          size += 28; break;
        case D3DFVF_XYZB5:          size += 32; break;
        default:                    size += 12; break;
    }
    if (fvf & D3DFVF_NORMAL)  size += 12;
    if (fvf & D3DFVF_DIFFUSE) size += 4;
    if (fvf & D3DFVF_SPECULAR) size += 4;
    size += ((fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT) * 8;
    return size;
}

/* canonical staged vertex */
struct CanonVert
{
    float    pos[4];      // location 0
    uint32_t diffuse;     // location 1
    uint32_t specular;    // location 2
    float    uv0[2];      // location 3
    float    uv1[2];      // location 4
    float    normal[4];   // location 5
};

struct DrawUBO
{
    float mvp[16];
    float params[4];
    float viewport[4];
    float material[4];
    float light[4];
    float lightDir[4];
    float ambient[4];
    float alphaRef[4];
    float stage[4];
};

/* ------------------------------------------------------------------ */
/* bridge singleton                                                    */
/* ------------------------------------------------------------------ */

struct Dx8Head { void** vtbl; volatile LONG refs; };

struct BufObj;
struct TexObj;
struct SurfObj;

struct DevObj;

struct Bridge
{
    /* Vulkan core (borrowed from VulkanDevice) */
    VulkanDevice vd;
    bool         ready = false;
    HWND         hwnd = NULL;
    uint32_t     width = 1280, height = 720;

    VkDevice     dev = VK_NULL_HANDLE;
    VkQueue      queue = VK_NULL_HANDLE;
    uint32_t     queueFamily = 0;

    /* frame machinery */
    static const int MAX_FRAMES = 3;
    VkFence        fences[MAX_FRAMES] = {};
    VkSemaphore    presentSem[MAX_FRAMES] = {};
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    int            frame = 0;          // current frame slot (advance on Present)
    bool           frameStarted = false;
    uint32_t       acquiredIndex = 0;
    VkResult       acquireResult = VK_SUCCESS;

    /* rings (host visible) */
    VkBuffer       ringBuf = VK_NULL_HANDLE;   // vertex/indice/UBO staging ring
    VkDeviceMemory ringMem = VK_NULL_HANDLE;
    void*          ringMap = nullptr;
    size_t         ringCap = 0;
    size_t         ringCursor = 0;
    size_t         ringFrameBase = 0;

    /* textures / resources */
    VkSampler      sampler = VK_NULL_HANDLE;
    VkImage        whiteImg = VK_NULL_HANDLE;
    VkDeviceMemory whiteMem = VK_NULL_HANDLE;
    VkImageView    whiteView = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
    VkDescriptorPool setPool = VK_NULL_HANDLE;
    VkPipelineCache  pipeCache = VK_NULL_HANDLE;

    /* renderpasses by (colorFmt,depthFmt) */
    std::map<std::pair<VkFormat,VkFormat>, VkRenderPass> renderPasses;
    VkBuffer canonRingIdxDummy = VK_NULL_HANDLE;

    /* pipeline cache key -> pipeline */
    struct PipeKey {
        VkStructureType _; uint64_t k;
        bool operator<(const PipeKey& o) const { return k < o.k; }
    };
    std::map<uint64_t, VkPipeline> pipelines;

    /* backbuffer/default depth */
    VkImage        defDepthImg = VK_NULL_HANDLE;
    VkDeviceMemory defDepthMem = VK_NULL_HANDLE;
    VkImageView    defDepthView = VK_NULL_HANDLE;
    VkFormat       defDepthFmt = VK_FORMAT_D24_UNORM_S8_UINT;
    uint32_t       defDepthW = 0, defDepthH = 0;

    /* framebuffers for swapchain images + depth */
    std::vector<VkFramebuffer> swapFbs;
    VkRenderPass swapRP = VK_NULL_HANDLE;

    /* current render target */
    VkFramebuffer curFb = VK_NULL_HANDLE;
    VkRenderPass  curRp = VK_NULL_HANDLE;
    uint32_t      curW = 0, curH = 0;
    bool          passOpen = false;
    bool          clearPending = false;
    VkClearValue  clearVals[2] = {};
    uint32_t      clearFlags = 0;      // D3DCLEAR_*
    bool          rtFirstUse = false;  // first pass on this FB in frame: clear it

    /* captured D3D state */
    uint32_t      rs[512];
    float         transforms[16][16];
    D3DMATERIAL8  material = {};
    D3DLIGHT8     lights[8] = {};
    float         lightDir[3] = {0,0,1};
    bool          lightEnable[8] = {};
    struct Stage { int op; int uv; DWORD colorOp; DWORD alphaOp; DWORD arg1; DWORD arg2; } stages[4] = {};
    TexObj*       bound[4] = {};
    BufObj*       stream = nullptr;
    UINT          streamStride = 0;
    BufObj*       ibuf = nullptr;
    DWORD         fvf = 0;
    VkViewport    vp = {};
    RECT          scissor = {};
    bool          scissorEnabled = false;
    D3DPRIMITIVETYPE lastPt = D3DPT_TRIANGLELIST;

    DevObj*       deviceObj = nullptr;   // current device (single)

    /* helpers */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props);
    void*    ringAlloc(size_t size, size_t* offsetOut);   // returns mapped pointer
    void     resetRing();
};

static Bridge* g = nullptr;

static void bridge_vk_assert(VkResult r, const char* what)
{
    if (r != VK_SUCCESS) dx8vk_log("FATAL %s -> %d", what, (int)r);
}

static VkImage create_image_linear(uint32_t w, uint32_t h, VkFormat fmt, VkImageUsageFlags usage, VkDeviceMemory* memOut)
{
    VkDevice dev = g->dev;
    VkImageCreateInfo ci = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = fmt;
    ci.extent = { w, h, 1 };
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_LINEAR;
    ci.usage = usage;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage img;
    bridge_vk_assert(vkCreateImage(dev, &ci, nullptr, &img), "create image");

    VkMemoryRequirements req; vkGetImageMemoryRequirements(dev, img, &req);
    VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = g->findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDeviceMemory mem;
    if (vkAllocateMemory(dev, &ai, nullptr, &mem) != VK_SUCCESS) {
        ai.memoryTypeIndex = g->findMemoryType(req.memoryTypeBits, 0);
        bridge_vk_assert(vkAllocateMemory(dev, &ai, nullptr, &mem), "image alloc");
    }
    vkBindImageMemory(dev, img, mem, 0);
    *memOut = mem;
    return img;
}

static void upload_image_via_staging(VkImage img, const void* src, size_t srcSize,
                                     uint32_t w, uint32_t h, uint32_t rowBytesPadded)
{
    /* record into current frame cmd buffer */
    if (!g->cmd) return;
    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageExtent = { w, h, 1 };
    region.imageOffset = { 0, 0, 0 };

    /* transition to general, copy, barrier back for sampling */
    VkImageMemoryBarrier toBar = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    toBar.srcAccessMask = 0; toBar.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toBar.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; toBar.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    toBar.image = img; toBar.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdPipelineBarrier(g->cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toBar);

    /* the ring needs a VkBuffer view: recreate-on-demand buffer wrapping not possible;
       use a dedicated upload buffer instead */
    static VkBuffer s_buf = VK_NULL_HANDLE;
    static VkDeviceMemory s_mem = VK_NULL_HANDLE;
    static VkDeviceSize s_cap = 0;
    if (s_cap < (VkDeviceSize)srcSize) {
        if (s_buf) { vkDestroyBuffer(g->dev, s_buf, nullptr); vkFreeMemory(g->dev, s_mem, nullptr); }
        VkBufferCreateInfo bi = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bi.size = (srcSize + 65535) & ~(VkDeviceSize)65535;
        bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        vkCreateBuffer(g->dev, &bi, nullptr, &s_buf);
        VkMemoryRequirements r; vkGetBufferMemoryRequirements(g->dev, s_buf, &r);
        VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = r.size;
        ai.memoryTypeIndex = g->findMemoryType(r.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkAllocateMemory(g->dev, &ai, nullptr, &s_mem);
        vkBindBufferMemory(g->dev, s_buf, s_mem, 0);
        s_cap = bi.size;
    }
    void* dst = nullptr;
    vkMapMemory(g->dev, s_mem, 0, srcSize, 0, &dst);
    memcpy(dst, src, srcSize);
    vkUnmapMemory(g->dev, s_mem);
    region.bufferOffset = 0;

    vkCmdCopyBufferToImage(g->cmd, s_buf, img, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

    VkImageMemoryBarrier rdBar = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    rdBar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    rdBar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    rdBar.oldLayout = rdBar.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    rdBar.image = img; rdBar.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdPipelineBarrier(g->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &rdBar);
}

/* ------------------------------------------------------------------ */
/* resource objects                                                    */
/* ------------------------------------------------------------------ */

struct BufObj {
    void** vtbl; volatile LONG refs;
    bool isIndex;
    UINT size;             // declared size in bytes
    D3DFORMAT indexFmt;    // for index buffers (D3DFMT_R16F / R32F)
    uint8_t* mirror;       // CPU side store (always kept; uploads when dirty)
    bool dirty;
};

struct SurfObj {
    void** vtbl; volatile LONG refs;
    TexObj* parentTex;     // may be null (standalone image surface / RT)
    UINT level;
    /* RT / depth-stencil backing */
    VkImage img; VkDeviceMemory mem; VkImageView view;
    uint32_t w, h;
    D3DFORMAT fmt;
    bool isDepth;
    VkFramebuffer fb;
    bool fbValid;
    uint8_t* mirror;       // for CPU image surfaces (CreateImageSurface)
};

struct TexObj {
    void** vtbl; volatile LONG refs;
    UINT w, h, levels;
    D3DFORMAT fmt;
    VkImage img; VkDeviceMemory mem; VkImageView view;
    uint8_t* mirror;       // level-0 CPU store (also all levels packed)
    size_t mirrorSize;
    bool dirty;            // re-upload needed
    bool isSurfaceHolder;  // created as render-target surface container
    SurfObj* level0Surf;
};

static void** vtbl_root();
static void** vtbl_dev();
static void** vtbl_vb();
static void** vtbl_ib();
static void** vtbl_tx();
static void** vtbl_sf();

/* ------------------------------------------------------------------ */
/* Vulkan plumbing                                                     */
/* ------------------------------------------------------------------ */

uint32_t Bridge::findMemoryType(uint32_t filter, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(vd.GetPhysicalDevice(), &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
        if ((filter & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props)
            return i;
    return 0;
}

void* Bridge::ringAlloc(size_t size, size_t* offsetOut)
{
    size = (size + 255) & ~(size_t)255;
    if (ringCursor + size > ringCap) {
        dx8vk_log_once("ring-overflow", "ring overflow (need %zu of %zu)", size, ringCap);
        *offsetOut = ringFrameBase;   // overwrite oldest data (best effort)
        return (char*)ringMap + ringFrameBase;
    }
    *offsetOut = ringCursor;
    ringCursor += size;
    return (char*)ringMap + *offsetOut;
}

void Bridge::resetRing() { ringCursor = ringFrameBase; }

static VkRenderPass getRenderPass(VkFormat colorFmt, VkFormat depthFmt)
{
    auto it = g->renderPasses.find({colorFmt, depthFmt});
    if (it != g->renderPasses.end()) return it->second;

    VkAttachmentDescription atts[2] = {};
    uint32_t n = 0;
    atts[n].format = colorFmt; atts[n].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[n].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; atts[n].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    atts[n].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; atts[n].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[n].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; atts[n].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    if (depthFmt != VK_FORMAT_UNDEFINED && colorFmt != g->vd.GetSwapchainFormat())
        atts[n].finalLayout = VK_IMAGE_LAYOUT_GENERAL;   // RT textures may be sampled later
    n++;
    if (depthFmt != VK_FORMAT_UNDEFINED) {
        atts[n].format = depthFmt; atts[n].samples = VK_SAMPLE_COUNT_1_BIT;
        atts[n].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; atts[n].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[n].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; atts[n].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[n].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; atts[n].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        n++;
    }
    VkAttachmentReference colorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub = {};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1; sub.pColorAttachments = &colorRef;
    if (n > 1) sub.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dep = {};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL; dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPass rp;
    VkRenderPassCreateInfo rpci = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    rpci.attachmentCount = n; rpci.pAttachments = atts;
    rpci.subpassCount = 1; rpci.pSubpasses = &sub;
    rpci.dependencyCount = 1; rpci.pDependencies = &dep;
    bridge_vk_assert(vkCreateRenderPass(g->dev, &rpci, nullptr, &rp), "renderpass");
    g->renderPasses[{colorFmt, depthFmt}] = rp;
    return rp;
}

static VkDescriptorSet allocTexSet(TexObj* t0, TexObj* t1, size_t uboRange)
{
    VkDescriptorSetAllocateInfo ai = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    ai.descriptorPool = g->setPool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &g->setLayout;
    VkDescriptorSet set;
    if (vkAllocateDescriptorSets(g->dev, &ai, &set) != VK_SUCCESS) return VK_NULL_HANDLE;

    VkDescriptorImageInfo ii[2];
    ii[0].sampler = g->sampler;
    ii[0].imageView = (t0 && t0->view) ? t0->view : g->whiteView;
    ii[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    ii[1].sampler = g->sampler;
    ii[1].imageView = (t1 && t1->view) ? t1->view : g->whiteView;
    ii[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet w = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    w.dstSet = set; w.dstBinding = 0; w.descriptorCount = 2;
    w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;   // sampler bound separately (binding 2)
    w.pImageInfo = ii;
    /* SAMPLED_IMAGE wants imageView+layout only; combined info entries are fine. */
    w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    vkUpdateDescriptorSets(g->dev, 1, &w, 0, nullptr);

    VkDescriptorImageInfo si = {};
    si.sampler = g->sampler; si.imageView = VK_NULL_HANDLE; si.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkWriteDescriptorSet w2 = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    w2.dstSet = set; w2.dstBinding = 2; w2.descriptorCount = 1;
    w2.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER; w2.pImageInfo = &si;
    vkUpdateDescriptorSets(g->dev, 1, &w2, 0, nullptr);

    VkDescriptorBufferInfo bi = {};
    bi.buffer = g->ringBuf; bi.offset = 0; bi.range = uboRange;
    VkWriteDescriptorSet w3 = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    w3.dstSet = set; w3.dstBinding = 1; w3.descriptorCount = 1;
    w3.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; w3.pBufferInfo = &bi;
    vkUpdateDescriptorSets(g->dev, 1, &w3, 0, nullptr);
    return set;
}

static VkPipeline buildPipeline(uint64_t key, VkRenderPass rp, VkPrimitiveTopology topo)
{
    auto it = g->pipelines.find(key);
    if (it != g->pipelines.end()) return it->second;

    VkShaderModule vs, fs;
    VkShaderModuleCreateInfo smci = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    smci.codeSize = sizeof(dx8bridge_vert_spv); smci.pCode = dx8bridge_vert_spv;
    vkCreateShaderModule(g->dev, &smci, nullptr, &vs);
    smci.codeSize = sizeof(dx8bridge_frag_spv); smci.pCode = dx8bridge_frag_spv;
    vkCreateShaderModule(g->dev, &smci, nullptr, &fs);

    const uint32_t stride = sizeof(CanonVert);
    VkVertexInputBindingDescription vbd = { 0, stride, VK_VERTEX_INPUT_RATE_VERTEX };
    VkVertexInputAttributeDescription vads[6] = {
        { 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, (uint32_t)offsetof(CanonVert, pos) },
        { 1, 0, VK_FORMAT_R32_UINT,             (uint32_t)offsetof(CanonVert, diffuse) },
        { 2, 0, VK_FORMAT_R32_UINT,             (uint32_t)offsetof(CanonVert, specular) },
        { 3, 0, VK_FORMAT_R32G32_SFLOAT,        (uint32_t)offsetof(CanonVert, uv0) },
        { 4, 0, VK_FORMAT_R32G32_SFLOAT,        (uint32_t)offsetof(CanonVert, uv1) },
        { 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT,  (uint32_t)offsetof(CanonVert, normal) },
    };
    VkPipelineVertexInputStateCreateInfo vi = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vi.vertexBindingDescriptionCount = 1; vi.pVertexBindingDescriptions = &vbd;
    vi.vertexAttributeDescriptionCount = 6; vi.pVertexAttributeDescriptions = vads;

    VkPipelineInputAssemblyStateCreateInfo ia = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    ia.topology = topo;

    VkPipelineViewportStateCreateInfo vsi = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    vsi.viewportCount = 1; vsi.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth = 1.0f;

    /* state decode from key */
    uint32_t cull = key & 7;
    uint32_t depthEn = (key >> 3) & 1, depthWr = (key >> 4) & 1, depthFn = (key >> 5) & 15;
    uint32_t blendEn = (key >> 9) & 1, srcF = (key >> 10) & 31, dstF = (key >> 15) & 31;
    uint32_t wire = (key >> 20) & 1;
    uint32_t alphaEn = (key >> 21) & 1;

    rs.cullMode = cull == 0 ? VK_CULL_MODE_NONE : (cull == 1 ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT);
    if (wire) { rs.polygonMode = VK_POLYGON_MODE_LINE; rs.lineWidth = 1.0f; }

    auto mapCmp = [](DWORD v) -> VkCompareOp {
        switch ((int)v) {
            case D3DCMP_NEVER:         return VK_COMPARE_OP_NEVER;
            case D3DCMP_LESS:          return VK_COMPARE_OP_LESS;
            case D3DCMP_EQUAL:         return VK_COMPARE_OP_EQUAL;
            case D3DCMP_LESSEQUAL:     return VK_COMPARE_OP_LESS_OR_EQUAL;
            case D3DCMP_GREATER:       return VK_COMPARE_OP_GREATER;
            case D3DCMP_NOTEQUAL:      return VK_COMPARE_OP_NOT_EQUAL;
            case D3DCMP_GREATEREQUAL:  return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case D3DCMP_ALWAYS:        return VK_COMPARE_OP_ALWAYS;
            default:                   return VK_COMPARE_OP_LESS;
        }
    };
    auto mapBlend = [](DWORD v) -> VkBlendFactor {
        switch ((int)v) {
            case D3DBLEND_ZERO:            return VK_BLEND_FACTOR_ZERO;
            case D3DBLEND_ONE:             return VK_BLEND_FACTOR_ONE;
            case D3DBLEND_SRCCOLOR:        return VK_BLEND_FACTOR_SRC_COLOR;
            case D3DBLEND_INVSRCCOLOR:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case D3DBLEND_SRCALPHA:        return VK_BLEND_FACTOR_SRC_ALPHA;
            case D3DBLEND_INVSRCALPHA:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case D3DBLEND_DESTALPHA:       return VK_BLEND_FACTOR_DST_ALPHA;
            case D3DBLEND_INVDESTALPHA:    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case D3DBLEND_DESTCOLOR:       return VK_BLEND_FACTOR_DST_COLOR;
            case D3DBLEND_INVDESTCOLOR:    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case D3DBLEND_SRCALPHASAT:     return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            default:                       return VK_BLEND_FACTOR_ONE;
        }
    };
    VkPipelineDepthStencilStateCreateInfo ds = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    ds.depthTestEnable = depthEn; ds.depthWriteEnable = depthWr;
    ds.depthCompareOp = mapCmp(depthFn);
    ds.stencilTestEnable = alphaEn && 0;

    VkPipelineColorBlendAttachmentState cba = {};
    cba.blendEnable = blendEn;
    cba.srcColorBlendFactor = mapBlend(srcF);
    cba.dstColorBlendFactor = mapBlend(dstF);
    cba.srcAlphaBlendFactor = cba.srcColorBlendFactor;
    cba.dstAlphaBlendFactor = cba.dstColorBlendFactor;
    cba.colorWriteMask = 0xF;
    VkPipelineColorBlendStateCreateInfo cb = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    cb.attachmentCount = 1; cb.pAttachments = &cba;

    VkPipelineMultisampleStateCreateInfo ms = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkDynamicState dyn[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dsi = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dsi.dynamicStateCount = 2; dsi.pDynamicStates = dyn;

    VkGraphicsPipelineCreateInfo gpi = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; stages[0].module = vs; stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; stages[1].module = fs; stages[1].pName = "main";
    gpi.stageCount = 2; gpi.pStages = stages;
    gpi.pVertexInputState = &vi; gpi.pInputAssemblyState = &ia;
    gpi.pViewportState = &vsi; gpi.pRasterizationState = &rs;
    gpi.pMultisampleState = &ms; gpi.pDepthStencilState = &ds;
    gpi.pColorBlendState = &cb; gpi.pDynamicState = &dsi;
    gpi.layout = g->pipeLayout; gpi.renderPass = rp; gpi.subpass = 0;

    VkPipeline pipe;
    VkResult r = vkCreateGraphicsPipelines(g->dev, g->pipeCache, 1, &gpi, nullptr, &pipe);
    vkDestroyShaderModule(g->dev, vs, nullptr);
    vkDestroyShaderModule(g->dev, fs, nullptr);
    if (r != VK_SUCCESS) { dx8vk_log("pipeline create failed %d", (int)r); pipe = VK_NULL_HANDLE; }
    g->pipelines[key] = pipe;
    return pipe;
}

/* ------------------------------------------------------------------ */
/* bridge lifecycle                                                    */
/* ------------------------------------------------------------------ */

static bool bridge_init(HWND hwnd, UINT w, UINT h)
{
    if (g && g->ready) return true;
    g = new Bridge();
    if (!g->vd.Initialize(hwnd, (int)w, (int)h)) { delete g; g = nullptr; return false; }
    g->dev = g->vd.GetDevice();
    g->queue = g->vd.GetGraphicsQueue();
    g->queueFamily = g->vd.GetGraphicsQueueFamily();
    g->hwnd = hwnd; g->width = w; g->height = h;

    /* sampler */
    VkSamplerCreateInfo sci = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sci.addressModeU = sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.minLod = 0; sci.maxLod = 0;
    bridge_vk_assert(vkCreateSampler(g->dev, &sci, nullptr, &g->sampler), "sampler");

    /* white 1x1 */
    VkDeviceMemory wm;
    g->whiteImg = create_image_linear(1, 1, VK_FORMAT_B8G8R8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &wm);
    g->whiteMem = wm;
    {
        /* initial upload through a one-shot transfer */
        uint32_t white = 0xFFFFFFFFu;
        VkBufferCreateInfo bi = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bi.size = 4; bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkBuffer buf; VkDeviceMemory mem;
        vkCreateBuffer(g->dev, &bi, nullptr, &buf);
        VkMemoryRequirements r; vkGetBufferMemoryRequirements(g->dev, buf, &r);
        VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = r.size;
        ai.memoryTypeIndex = g->findMemoryType(r.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkAllocateMemory(g->dev, &ai, nullptr, &mem);
        vkBindBufferMemory(g->dev, buf, mem, 0);
        void* p; vkMapMemory(g->dev, mem, 0, 4, 0, &p); memcpy(p, &white, 4); vkUnmapMemory(g->dev, mem);

        VkCommandBufferAllocateInfo cai = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cai.commandPool = g->vd.GetCommandPool(); cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cai.commandBufferCount = 1;
        VkCommandBuffer c; vkAllocateCommandBuffers(g->dev, &cai, &c);
        VkCommandBufferBeginInfo beg = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beg.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(c, &beg);
        VkImageMemoryBarrier b = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        b.srcAccessMask = 0; b.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        b.image = g->whiteImg; b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &b);
        VkBufferImageCopy reg = {};
        reg.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        reg.imageExtent = { 1, 1, 1 };
        vkCmdCopyBufferToImage(c, buf, g->whiteImg, VK_IMAGE_LAYOUT_GENERAL, 1, &reg);
        VkImageMemoryBarrier b2 = b;
        b2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; b2.dstAccessMask = 0;
        b2.oldLayout = b2.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        vkCmdPipelineBarrier(c, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &b2);
        vkEndCommandBuffer(c);
        VkSubmitInfo si = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        si.commandBufferCount = 1; si.pCommandBuffers = &c;
        vkQueueSubmit(g->queue, 1, &si, VK_NULL_HANDLE);
        vkQueueWaitIdle(g->queue);
        vkFreeCommandBuffers(g->dev, g->vd.GetCommandPool(), 1, &c);
        vkDestroyBuffer(g->dev, buf, nullptr);
        vkFreeMemory(g->dev, mem, nullptr);
    }
    {
        VkImageViewCreateInfo vci = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vci.image = g->whiteImg; vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = VK_FORMAT_B8G8R8A8_UNORM;
        vci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCreateImageView(g->dev, &vci, nullptr, &g->whiteView);
    }

    /* descriptor set layout: 0 sampled[2], 1 ubo dynamic, 2 sampler */
    VkDescriptorSetLayoutBinding bds[3] = {
        { 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { 2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
    };
    VkDescriptorSetLayoutCreateInfo dli = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    dli.bindingCount = 3; dli.pBindings = bds;
    vkCreateDescriptorSetLayout(g->dev, &dli, nullptr, &g->setLayout);

    VkPipelineLayoutCreateInfo pli = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pli.setLayoutCount = 1; pli.pSetLayouts = &g->setLayout;
    vkCreatePipelineLayout(g->dev, &pli, nullptr, &g->pipeLayout);

    VkDescriptorPoolSize poolSizes[3] = {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024 * 2 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
    };
    VkDescriptorPoolCreateInfo dpi = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    dpi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpi.maxSets = 1024; dpi.poolSizeCount = 3; dpi.pPoolSizes = poolSizes;
    vkCreateDescriptorPool(g->dev, &dpi, nullptr, &g->setPool);

    VkPipelineCacheCreateInfo pcci = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    vkCreatePipelineCache(g->dev, &pcci, nullptr, &g->pipeCache);

    /* unified ring: 32 MB */
    {
        VkBufferCreateInfo bi = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bi.size = 32u << 20;
        bi.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bridge_vk_assert(vkCreateBuffer(g->dev, &bi, nullptr, &g->ringBuf), "ring buffer");
        VkMemoryRequirements r; vkGetBufferMemoryRequirements(g->dev, g->ringBuf, &r);
        VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = r.size;
        ai.memoryTypeIndex = g->findMemoryType(r.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        bridge_vk_assert(vkAllocateMemory(g->dev, &ai, nullptr, &g->ringMem), "ring mem");
        vkBindBufferMemory(g->dev, g->ringBuf, g->ringMem, 0);
        vkMapMemory(g->dev, g->ringMem, 0, bi.size, 0, &g->ringMap);
        g->ringCap = bi.size;
        g->ringFrameBase = 0; g->ringCursor = 0;
    }

    for (int i = 0; i < Bridge::MAX_FRAMES; i++) {
        VkFenceCreateInfo fci = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(g->dev, &fci, nullptr, &g->fences[i]);
        VkSemaphoreCreateInfo sei = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(g->dev, &sei, nullptr, &g->presentSem[i]);
    }

    /* default depth + swap framebuffers */
    g->ready = true;
    return true;
}

static void ensure_default_depth(uint32_t w, uint32_t h)
{
    if (g->defDepthImg && g->defDepthW == w && g->defDepthH == h) return;
    if (g->defDepthImg) {
        vkDestroyImageView(g->dev, g->defDepthView, nullptr);
        vkDestroyImage(g->dev, g->defDepthImg, nullptr);
        vkFreeMemory(g->dev, g->defDepthMem, nullptr);
        g->defDepthImg = VK_NULL_HANDLE;
    }
    VkImageCreateInfo ci = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ci.imageType = VK_IMAGE_TYPE_2D; ci.format = g->defDepthFmt;
    ci.extent = { w, h, 1 }; ci.mipLevels = 1; ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT; ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkCreateImage(g->dev, &ci, nullptr, &g->defDepthImg);
    VkMemoryRequirements r; vkGetImageMemoryRequirements(g->dev, g->defDepthImg, &r);
    VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    ai.allocationSize = r.size;
    ai.memoryTypeIndex = g->findMemoryType(r.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkAllocateMemory(g->dev, &ai, nullptr, &g->defDepthMem);
    vkBindImageMemory(g->dev, g->defDepthImg, g->defDepthMem, 0);
    VkImageViewCreateInfo vci = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vci.image = g->defDepthImg; vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = g->defDepthFmt;
    vci.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
    vkCreateImageView(g->dev, &vci, nullptr, &g->defDepthView);
    g->defDepthW = w; g->defDepthH = h;
    g->swapRP = VK_NULL_HANDLE;
    for (size_t i = 0; i < g->swapFbs.size(); i++) vkDestroyFramebuffer(g->dev, g->swapFbs[i], nullptr);
    g->swapFbs.clear();
}

static void ensure_swap_fbs()
{
    uint32_t n = g->vd.GetSwapchainImageCount();
    VkExtent2D ext = g->vd.GetSwapchainExtent();
    ensure_default_depth(ext.width ? ext.width : g->width, ext.height ? ext.height : g->height);
    if (g->swapRP == VK_NULL_HANDLE)
        g->swapRP = getRenderPass(g->vd.GetSwapchainFormat(), g->defDepthFmt);
    if (!g->swapFbs.empty()) return;
    for (uint32_t i = 0; i < n; i++) {
        VkImageView color = g->vd.GetSwapchainImageView(i);
        VkFramebuffer fb;
        VkImageView atts[2] = { color, g->defDepthView };
        VkFramebufferCreateInfo fci = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fci.renderPass = g->swapRP; fci.attachmentCount = 2; fci.pAttachments = atts;
        fci.width = ext.width; fci.height = ext.height; fci.layers = 1;
        vkCreateFramebuffer(g->dev, &fci, nullptr, &fb);
        g->swapFbs.push_back(fb);
    }
}

/* ------------------------------------------------------------------ */
/* resource COM implementations                                        */
/* ------------------------------------------------------------------ */

static TexObj* tex_from_iface(void* p) { return (TexObj*)p; }

static HRESULT dx8rb_QueryInterface(void* self, const void* riid, void** ppv)
{ (void)riid; if (!ppv) return E_INVALIDARG; *ppv = self; InterlockedIncrement(&((Dx8Head*)self)->refs); return S_OK; }

static ULONG dx8rb_AddRef(void* self) { return (ULONG)InterlockedIncrement(&((Dx8Head*)self)->refs); }
static ULONG dx8rb_Release(void* self)
{
    ULONG r = (ULONG)InterlockedDecrement(&((Dx8Head*)self)->refs);
    return r;   /* bridge objects live until device shutdown; leak is bounded */
}


static HRESULT dx8vb_QueryInterface(void* self, const void* riid, void** ppv)
{
    (void)riid;
    if (!ppv) return E_INVALIDARG;
    *ppv = self;
    InterlockedIncrement(&((Dx8Head*)self)->refs);
    return S_OK;
}
static ULONG dx8vb_AddRef(void* self) { return (ULONG)InterlockedIncrement(&((Dx8Head*)self)->refs); }
static ULONG dx8vb_Release(void* self) { return (ULONG)InterlockedDecrement(&((Dx8Head*)self)->refs); }
static HRESULT dx8vb_GetDevice(void* self, void** pp)
{ if (!pp) return D3DERR_INVALIDCALL; *pp = g->deviceObj; InterlockedIncrement(&((Dx8Head*)g->deviceObj)->refs); return D3D_OK; }
static HRESULT dx8vb_SetPrivateData(void* self, const void* c, void* d, DWORD s) { (void)self; (void)c; (void)d; (void)s; return D3D_OK; }
static HRESULT dx8vb_GetPrivateData(void* self, const void* c, void* d, DWORD* s) { (void)self; (void)c; (void)d; if (s) *s = 0; return D3D_OK; }
static HRESULT dx8vb_SetPriority(void* self, DWORD p) { (void)self; (void)p; return D3D_OK; }
static DWORD dx8vb_GetPriority(void* self) { (void)self; return 0; }
static void dx8vb_PreLoad(void* self) { (void)self; }
static D3DRESOURCETYPE dx8vb_GetType(void* self) { (void)self; return D3DRTYPE_VERTEXBUFFER; }

static HRESULT dx8vb_Lock(void* self, UINT offset, UINT size, void** ppData, DWORD flags)
{
    BufObj* b = (BufObj*)self;
    (void)flags;
    if (!ppData) return D3DERR_INVALIDCALL;
    if (offset + size > b->size) size = b->size - offset;
    *ppData = b->mirror + offset;
    b->dirty = true;
    return D3D_OK;
}
static HRESULT dx8vb_Unlock(void* self) { (void)self; return D3D_OK; }
static HRESULT dx8vb_GetDesc(void* self, void* pDesc)
{
    BufObj* b = (BufObj*)self;
    D3DVERTEXBUFFER_DESC* d = (D3DVERTEXBUFFER_DESC*)pDesc;
    if (!d) return D3DERR_INVALIDCALL;
    memset(d, 0, sizeof(*d));
    d->Format = b->isIndex ? D3DFMT_INDEX16 : (D3DFORMAT)0;
    d->Type = b->isIndex ? D3DRTYPE_INDEXBUFFER : D3DRTYPE_VERTEXBUFFER;
    d->Usage = 0; d->Pool = D3DPOOL_DEFAULT; d->Size = b->size; d->FVF = 0;
    return D3D_OK;
}

static HRESULT dx8ib_Lock(void* self, UINT offset, UINT size, void** ppData, DWORD flags)
{ return dx8vb_Lock(self, offset, size, ppData, flags); }
static HRESULT dx8ib_Unlock(void* self) { return dx8vb_Unlock(self); }
static HRESULT dx8ib_GetDesc(void* self, void* pDesc)
{
    BufObj* b = (BufObj*)self;
    D3DINDEXBUFFER_DESC* d = (D3DINDEXBUFFER_DESC*)pDesc;
    if (!d) return D3DERR_INVALIDCALL;
    memset(d, 0, sizeof(*d));
    d->Format = b->indexFmt; d->Type = D3DRTYPE_INDEXBUFFER;
    d->Usage = 0; d->Pool = D3DPOOL_DEFAULT; d->Size = b->size;
    return D3D_OK;
}
/* shared trivial impls for index/texture/surface (same layout) */
#define RES_TRIVIAL(pfx, typeval) \
static HRESULT pfx##_QueryInterface(void* s, const void* riid, void** pp){ return dx8vb_QueryInterface(s, riid, pp); } \
static ULONG pfx##_AddRef(void* s){ return dx8vb_AddRef(s); } \
static ULONG pfx##_Release(void* s){ return dx8vb_Release(s); } \
static HRESULT pfx##_GetDevice(void* s, void** pp){ return dx8vb_GetDevice(s, pp); } \
static HRESULT pfx##_SetPrivateData(void* s, const void* a, void* b, DWORD c){ return dx8vb_SetPrivateData(s,a,b,c);} \
static HRESULT pfx##_GetPrivateData(void* s, const void* a, void* b, DWORD* c){ return dx8vb_GetPrivateData(s,a,b,c);} \
static HRESULT pfx##_SetPriority(void* s, DWORD p){ return dx8vb_SetPriority(s,p);} \
static DWORD pfx##_GetPriority(void* s){ return dx8vb_GetPriority(s);} \
static void pfx##_PreLoad(void* s){ dx8vb_PreLoad(s);} \
static D3DRESOURCETYPE pfx##_GetType(void* s){ (void)s; return typeval; }
RES_TRIVIAL(dx8ib, D3DRTYPE_VERTEXBUFFER+1)
RES_TRIVIAL(dx8tx, D3DRTYPE_TEXTURE)
RES_TRIVIAL(dx8sf, D3DRTYPE_SURFACE)

/* ------------------------------------------------------------------ */
/* texture & surface implementations                                   */
/* ------------------------------------------------------------------ */

static void tex_ensure_backing(TexObj* t)
{
    if (t->img) return;
    VkFormat fmt = vk_format_for(t->fmt);
    if (fmt == VK_FORMAT_UNDEFINED) { dx8vk_log_once("texfmt","unsupported texture fmt %d", (int)t->fmt); return; }
    VkDeviceMemory m;
    t->img = create_image_linear(t->w ? t->w : 1, t->h ? t->h : 1, fmt,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m);
    t->mem = m;
    VkImageViewCreateInfo vci = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    vci.image = t->img; vci.viewType = VK_IMAGE_VIEW_TYPE_2D; vci.format = fmt;
    vci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCreateImageView(g->dev, &vci, nullptr, &t->view);
}

static HRESULT dx8tx_GetLevelDesc(void* self, UINT level, void* pDesc)
{
    TexObj* t = (TexObj*)self;
    D3DSURFACE_DESC* d = (D3DSURFACE_DESC*)pDesc;
    if (!d) return D3DERR_INVALIDCALL;
    memset(d, 0, sizeof(*d));
    d->Format = t->fmt; d->Type = D3DRTYPE_SURFACE; d->Usage = 0;
    d->Pool = D3DPOOL_DEFAULT;
    UINT l = level; if (l >= t->levels) l = t->levels ? t->levels - 1 : 0;
    d->Width = (t->w >> l) ? (t->w >> l) : 1;
    d->Height = (t->h >> l) ? (t->h >> l) : 1;
    return D3D_OK;
}
static HRESULT dx8tx_SetLOD(void* s, DWORD l) { (void)s; (void)l; return D3D_OK; }
static DWORD dx8tx_GetLOD(void* s) { (void)s; return 0; }
static DWORD dx8tx_GetLevelCount(void* s) { return ((TexObj*)s)->levels; }

static HRESULT dx8tx_LockRect(void* self, UINT level, void* pLocked, const RECT* rect, DWORD flags)
{
    TexObj* t = (TexObj*)self;
    D3DLOCKED_RECT* lr = (D3DLOCKED_RECT*)pLocked;
    if (!lr) return D3DERR_INVALIDCALL;
    if (!t->mirror) {
        UINT w = t->w ? t->w : 1, h = t->h ? t->h : 1;
        size_t bytes;
        if (d3dfmt_is_compressed(t->fmt))
            bytes = ((w + 3) / 4) * ((h + 3) / 4) * ((t->fmt == D3DFMT_DXT1 || t->fmt == D3DFMT_DXT2) ? 8 : 16);
        else
            bytes = (size_t)w * h * d3dfmt_bpp(t->fmt);
        t->mirrorSize = bytes;
        t->mirror = (uint8_t*)calloc(1, bytes ? bytes : 1);
    }
    if (level != 0) {   /* higher mips unsupported: return 4x4 dummy region */
        lr->Pitch = 4;
        static uint8_t s_dummy[16];
        lr->pBits = s_dummy;
        return D3D_OK;
    }
    UINT pitch = d3dfmt_is_compressed(t->fmt)
        ? ((t->w + 3) / 4) * ((t->fmt == D3DFMT_DXT1 || t->fmt == D3DFMT_DXT2) ? 8 : 16)
        : t->w * d3dfmt_bpp(t->fmt);
    if (pitch == 0) pitch = 4;
    lr->Pitch = pitch;
    uint8_t* base = t->mirror;
    if (rect) base += (size_t)rect->top * pitch;
    lr->pBits = base;
    (void)flags;
    t->dirty = true;
    return D3D_OK;
}
static HRESULT dx8tx_UnlockRect(void* self) { (void)self; return D3D_OK; }
static HRESULT dx8tx_AddDirtyRect(void* s, const RECT* r) { ((TexObj*)s)->dirty = true; (void)r; return D3D_OK; }
static HRESULT dx8tx_GetSurfaceLevel(void* self, UINT level, void** ppSurface);

static HRESULT dx8sf_GetContainer(void* self, const void* riid, void** ppCont)
{
    SurfObj* sf = (SurfObj*)self;
    if (!ppCont) return D3DERR_INVALIDCALL;
    if (sf->parentTex) {
        InterlockedIncrement(&((Dx8Head*)sf->parentTex)->refs);
        *ppCont = sf->parentTex;
        return D3D_OK;
    }
    *ppCont = nullptr;
    return D3DERR_NOTFOUND;
}
static HRESULT dx8sf_FreePrivateData(void* s, const void* a) { (void)s; (void)a; return D3D_OK; }
static HRESULT dx8sf_GetDesc(void* self, void* pDesc)
{
    SurfObj* sf = (SurfObj*)self;
    D3DSURFACE_DESC* d = (D3DSURFACE_DESC*)pDesc;
    if (!d) return D3DERR_INVALIDCALL;
    memset(d, 0, sizeof(*d));
    d->Format = sf->fmt; d->Type = D3DRTYPE_SURFACE;
    d->Pool = D3DPOOL_DEFAULT;
    d->Width = sf->w ? sf->w : 1; d->Height = sf->h ? sf->h : 1;
    return D3D_OK;
}
static HRESULT dx8sf_LockRect(void* self, void* pLocked, const RECT* rect, DWORD flags)
{
    SurfObj* sf = (SurfObj*)self;
    D3DLOCKED_RECT* lr = (D3DLOCKED_RECT*)pLocked;
    if (!lr) return D3DERR_INVALIDCALL;
    if (sf->parentTex) return dx8tx_LockRect(sf->parentTex, sf->level, lr, rect, flags);
    if (!sf->mirror) {
        sf->mirror = (uint8_t*)calloc(1, (size_t)sf->w * sf->h * d3dfmt_bpp(sf->fmt) + 16);
    }
    lr->Pitch = sf->w * d3dfmt_bpp(sf->fmt);
    lr->pBits = sf->mirror;
    (void)rect; (void)flags;
    return D3D_OK;
}
static HRESULT dx8sf_UnlockRect(void* self) { (void)self; return D3D_OK; }
static HRESULT dx8sf_GetLevelDesc(void* self, UINT level, void* pDesc)
{
    SurfObj* sf = (SurfObj*)self;
    if (sf->parentTex) return dx8tx_GetLevelDesc(sf->parentTex, level, pDesc);
    return dx8sf_GetDesc(self, pDesc);
}

/* ------------------------------------------------------------------ */
/* device implementation                                               */
/* ------------------------------------------------------------------ */

struct DevObj { void** vtbl; volatile LONG refs; };

static void mat4_mul(const float a[16], const float b[16], float out[16])
{
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            out[r * 4 + c] = a[r*4+0]*b[0*4+c] + a[r*4+1]*b[1*4+c] + a[r*4+2]*b[2*4+c] + a[r*4+3]*b[3*4+c];
}

/* D3D row-major W*V*P -> GLSL column-major */
static void build_mvp(float out[16])
{
    float tmp[16], wp[16];
    mat4_mul(g->transforms[D3DTS_WORLD], g->transforms[D3DTS_VIEW], tmp);
    mat4_mul(tmp, g->transforms[D3DTS_PROJECTION], wp);
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            out[c * 4 + r] = wp[r * 4 + c];
}

static int normalize_stage_op(int stageIdx)
{
    Bridge::Stage* st = &g->stages[stageIdx];
    if (st->colorOp == D3DTOP_DISABLE || !g->bound[stageIdx]) return 0;  /* PASS */
    switch (st->colorOp) {
        case D3DTOP_MODULATE:        return 1;
        case D3DTOP_SELECTARG1:
            return (st->arg1 == D3DTA_TEXTURE) ? 2 : (st->arg1 == D3DTA_CURRENT ? 0 : 1);
        case D3DTOP_SELECTARG2:     return (st->arg2 == D3DTA_TEXTURE) ? 2 : 0;
        case D3DTOP_ADD:
        case D3DTOP_ADDSIGNED:
        case D3DTOP_ADDSIGNED2X:
        case D3DTOP_ADDSMOOTH:      return 3;
        case D3DTOP_BLENDTEXTUREALPHA:
        case D3DTOP_BLENDCURRENTALPHA:
        case D3DTOP_BLENDFACTORALPHA: return 4;
        default:
            dx8vk_log_once("stageop", "stage op %d unmapped -> MODULATE", (int)st->colorOp);
            return 1;
    }
}

static void tex_touch(TexObj* t)
{
    if (t && t->dirty) {
        tex_ensure_backing(t);
        if (t->img && t->mirror) {
            UINT pitch = d3dfmt_is_compressed(t->fmt)
                ? ((t->w + 3) / 4) * ((t->fmt == D3DFMT_DXT1 || t->fmt == D3DFMT_DXT2) ? 8 : 16)
                : t->w * d3dfmt_bpp(t->fmt);
            uint32_t h = t->h ? t->h : 1;
            upload_image_via_staging(t->img, t->mirror, (size_t)pitch * h, t->w ? t->w : 1, h, pitch);
        }
        t->dirty = false;
    }
}

static void flush_bound_textures();

/* begin a fresh renderpass on target fb */
static void begin_pass(VkFramebuffer fb, VkRenderPass rp, uint32_t w, uint32_t h, bool withClear)
{
    if (g->passOpen) vkCmdEndRenderPass(g->cmd);
    VkRenderPassBeginInfo bi = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    bi.renderPass = rp; bi.framebuffer = fb;
    bi.renderArea = { {0,0},{ w, h } };
    VkClearValue cv[2];
    if (withClear) {
        cv[0] = g->clearVals[0];
        cv[1].depthStencil.depth = g->clearVals[1].depthStencil.depth;
        cv[1].depthStencil.stencil = g->clearVals[1].depthStencil.stencil;
        bi.clearValueCount = 2;
        bi.pClearValues = cv;
    } else {
        bi.clearValueCount = 0;
    }
    flush_bound_textures();
    vkCmdBeginRenderPass(g->cmd, &bi, VK_SUBPASS_CONTENTS_INLINE);
    g->passOpen = true;
    g->curFb = fb; g->curRp = rp; g->curW = w; g->curH = h;
    g->vp.x = 0; g->vp.y = 0; g->vp.width = (float)w; g->vp.height = (float)h;
    g->vp.minDepth = 0; g->vp.maxDepth = 1;
    vkCmdSetViewport(g->cmd, 0, 1, &g->vp);
    VkRect2D sc = { {0,0},{ w, h } };
    vkCmdSetScissor(g->cmd, 0, 1, &sc);
}

static void ensure_frame_started()
{
    if (g->frameStarted) return;
    vkWaitForFences(g->dev, 1, &g->fences[g->frame], VK_TRUE, UINT64_MAX);
    vkResetFences(g->dev, 1, &g->fences[g->frame]);
    vkResetCommandBuffer(g->cmd, 0);
    g->resetRing();
    VkCommandBufferBeginInfo beg = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(g->cmd, &beg);
    g->acquireResult = vkAcquireNextImageKHR(g->dev, g->vd.GetSwapchain(), UINT64_MAX,
                                             g->presentSem[g->frame], VK_NULL_HANDLE, &g->acquiredIndex);
    g->frameStarted = true;
}

static void default_target(VkFramebuffer* fb, VkRenderPass* rp, uint32_t* w, uint32_t* h)
{
    ensure_swap_fbs();
    *fb = g->swapFbs[g->acquiredIndex % (uint32_t)g->swapFbs.size()];
    *rp = g->swapRP;
    VkExtent2D e = g->vd.GetSwapchainExtent();
    *w = e.width; *h = e.height;
}

static void rt_for_surface(void* surf, VkFramebuffer* fb, VkRenderPass* rp, uint32_t* w, uint32_t* h)
{
    if (!surf) { default_target(fb, rp, w, h); return; }
    SurfObj* sf = (SurfObj*)surf;
    VkFormat cf = vk_format_for(sf->fmt);
    if (cf == VK_FORMAT_UNDEFINED) { default_target(fb, rp, w, h); return; }
    if (sf->fb) { *fb = sf->fb; *rp = sf->fbValid ? getRenderPass(cf, g->defDepthFmt) : g->curRp; *w = sf->w; *h = sf->h; return; }
    if (!sf->img) { dx8vk_log_once("rt-notex", "SetRenderTarget to non-renderable -> backbuffer"); default_target(fb, rp, w, h); return; }
    VkRenderPass rpass = getRenderPass(cf, g->defDepthFmt);
    VkImageView atts[2] = { sf->view, g->defDepthView };
    VkFramebufferCreateInfo fci = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fci.renderPass = rpass; fci.attachmentCount = 2; fci.pAttachments = atts;
    fci.width = sf->w; fci.height = sf->h; fci.layers = 1;
    bridge_vk_assert(vkCreateFramebuffer(g->dev, &fci, nullptr, &sf->fb), "rt fb");
    sf->fbValid = true;
    *fb = sf->fb; *rp = rpass; *w = sf->w; *h = sf->h;
}

/* the gather-based draw */
static void flush_bound_textures() { tex_touch(g->bound[0]); tex_touch(g->bound[1]); }

static void emit_draw(D3DPRIMITIVETYPE pt, uint32_t primCount,
                      const uint32_t* indexBase, BufObj* idxObj, uint32_t indexStartElem, uint32_t indexCount,
                      uint32_t vtxBase, uint32_t vtxCountHint)
{
    if (!g->stream || !g->cmd) return;
    DWORD fvf = g->fvf;
    uint32_t stride = g->streamStride ? g->streamStride : fvf_size(fvf);
    if (stride == 0) return;

    bool hasPos   = true, isRHW = (fvf & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW;
    int  posKind  = (fvf & D3DFVF_POSITION_MASK);
    bool hasNrm = (fvf & D3DFVF_NORMAL) != 0;
    bool hasDif = (fvf & D3DFVF_DIFFUSE) != 0;
    bool hasSpc = (fvf & D3DFVF_SPECULAR) != 0;
    int  nUV    = (int)((fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT);
    if (nUV > 4) nUV = 4;

    uint32_t outCount = 0;
    switch (pt) {
        case D3DPT_TRIANGLELIST:  outCount = 3 * primCount; break;
        case D3DPT_TRIANGLESTRIP: outCount = primCount + 2; break;
        case D3DPT_TRIANGLEFAN:   outCount = primCount + 2; break;
        case D3DPT_LINELIST:      outCount = 2 * primCount; break;
        case D3DPT_LINESTRIP:     outCount = primCount + 1; break;
        case D3DPT_POINTLIST:     outCount = primCount; break;
        default: return;
    }
    if (idxObj && indexCount) outCount = indexCount;

    size_t need = (size_t)outCount * sizeof(CanonVert);
    size_t coff; void* cbase = g->ringAlloc(need, &coff);
    if (!cbase) return;
    CanonVert* outv = (CanonVert*)cbase;
    memset(outv, 0, need);

    const uint8_t* src = g->stream->mirror;
    for (uint32_t k = 0; k < outCount; k++) {
        uint32_t v = vtxBase + k;
        if (idxObj) {
            uint32_t idxv;
            const uint8_t* ib = idxObj->mirror;
            uint32_t e = indexStartElem + k;
            if (idxObj->indexFmt != D3DFMT_INDEX32) {
                idxv = ((const uint16_t*)ib)[e];
            } else {
                idxv = ((const uint32_t*)ib)[e];
            }
            v = idxv;
        }
        const uint8_t* p = src + (size_t)v * stride;
        CanonVert* o = &outv[k];
        if (isRHW) {
            o->pos[0] = *(const float*)(p + 0);
            o->pos[1] = *(const float*)(p + 4);
            o->pos[2] = *(const float*)(p + 8);
            o->pos[3] = *(const float*)(p + 12);
        } else {
            o->pos[0] = *(const float*)(p + 0);
            o->pos[1] = *(const float*)(p + 4);
            o->pos[2] = *(const float*)(p + 8);
            o->pos[3] = 1.0f;
        }
        size_t off = (posKind == D3DFVF_XYZ) ? 12 : (posKind == D3DFVF_XYZRHW ? 16 : (size_t)(3 + (posKind - D3DFVF_XYZB1)) * 4);
        if (hasNrm) { o->normal[0] = *(const float*)(p + off); o->normal[1] = *(const float*)(p + off + 4); o->normal[2] = *(const float*)(p + off + 8); o->normal[3] = 0; off += 12; }
        else { o->normal[0] = 0; o->normal[1] = 0; o->normal[2] = 1; o->normal[3] = 0; }
        if (hasDif) { o->diffuse = *(const uint32_t*)(p + off); off += 4; } else o->diffuse = 0xFFFFFFFFu;
        if (hasSpc) { o->specular = *(const uint32_t*)(p + off); off += 4; } else o->specular = 0;
        size_t uvoff[4] = {0, 0, 0, 0};
        for (int u = 0; u < nUV && u < 4; u++) { uvoff[u] = off; off += 8; }
        if (nUV > 0) { o->uv0[0] = *(const float*)(p + uvoff[0]); o->uv0[1] = *(const float*)(p + uvoff[0] + 4); }
        if (nUV > 1) { o->uv1[0] = *(const float*)(p + uvoff[1]); o->uv1[1] = *(const float*)(p + uvoff[1] + 4); }
    }

    /* uniform block */
    size_t uoff; void* udst = g->ringAlloc((sizeof(DrawUBO) + 255) & ~(size_t)255, &uoff);
    if (!udst) return;
    DrawUBO* u = (DrawUBO*)udst;
    memset(u, 0, sizeof(*u));
    build_mvp(u->mvp);
    u->params[0] = isRHW ? 1.0f : 0.0f;
    u->params[1] = g->rs[D3DRS_LIGHTING] ? 1.0f : 0.0f;
    u->viewport[0] = (float)(g->curW ? g->curW : 1);
    u->viewport[1] = (float)(g->curH ? g->curH : 1);
    u->material[0] = g->material.Diffuse.r; u->material[1] = g->material.Diffuse.g;
    u->material[2] = g->material.Diffuse.b; u->material[3] = g->material.Diffuse.a;
    int lidx = -1;
    for (int i = 0; i < 8; i++)
        if (g->lightEnable[i] && (g->lights[i].Type == D3DLIGHT_DIRECTIONAL || g->lights[i].Type == D3DLIGHT_SPOT)) { lidx = i; break; }
    if (lidx >= 0) {
        const D3DLIGHT8* L = &g->lights[lidx];
        u->light[0] = L->Diffuse.r; u->light[1] = L->Diffuse.g; u->light[2] = L->Diffuse.b;
        u->light[3] = 1.0f;
        (void)0;
        /* Direction used by the shader as light direction (intensity baked in Diffuse) */
        u->lightDir[0] = L->Direction.x; u->lightDir[1] = L->Direction.y; u->lightDir[2] = L->Direction.z;
    } else { u->light[3] = 0.0f; }
    uint32_t amb = g->rs[D3DRS_AMBIENT];
    u->ambient[0] = ((amb >> 16) & 0xff) / 255.0f;
    u->ambient[1] = ((amb >> 8) & 0xff) / 255.0f;
    u->ambient[2] = (amb & 0xff) / 255.0f;
    uint32_t matAmb = ((uint32_t)(g->material.Ambient.r*255)) | ((uint32_t)(g->material.Ambient.g*255)<<8) | ((uint32_t)(g->material.Ambient.b*255)<<16);
    u->ambient[0] *= ((matAmb>>16)&0xff)/255.0f*4.0f; u->ambient[1] *= ((matAmb>>8)&0xff)/255.0f*4.0f; u->ambient[2] *= (matAmb&0xff)/255.0f*4.0f;
    u->alphaRef[0] = g->rs[D3DRS_ALPHATESTENABLE] ? 1.0f : 0.0f;
    u->alphaRef[1] = (g->rs[D3DRS_ALPHAREF] & 0xff) / 255.0f;
    u->stage[0] = (float)normalize_stage_op(0);
    u->stage[1] = (float)normalize_stage_op(1);
    u->stage[2] = (float)g->stages[0].uv;
    u->stage[3] = (float)g->stages[1].uv;

    /* textures: bind set */
    uint64_t key = 0;
    key |= (uint64_t)(g->rs[D3DRS_CULLMODE] & 7);
    key |= (uint64_t)(g->rs[D3DRS_ZENABLE] ? 1 : 0) << 3;
    key |= (uint64_t)(g->rs[D3DRS_ZWRITEENABLE] ? 1 : 0) << 4;
    key |= (uint64_t)(g->rs[D3DRS_ZFUNC] & 15) << 5;
    key |= (uint64_t)(g->rs[D3DRS_ALPHABLENDENABLE] ? 1 : 0) << 9;
    key |= (uint64_t)(g->rs[D3DRS_SRCBLEND] & 31) << 10;
    key |= (uint64_t)(g->rs[D3DRS_DESTBLEND] & 31) << 15;
    key |= (uint64_t)((g->rs[D3DRS_FILLMODE] == D3DFILL_WIREFRAME) ? 1 : 0) << 20;
    key |= (uint64_t)(g->rs[D3DRS_ALPHATESTENABLE] ? 1 : 0) << 21;
    switch (pt) {
        case D3DPT_TRIANGLELIST: key |= 1ULL << 56; break;
        case D3DPT_TRIANGLESTRIP: key |= 2ULL << 56; break;
        case D3DPT_LINELIST: key |= 3ULL << 56; break;
        case D3DPT_LINESTRIP: key |= 4ULL << 56; break;
        case D3DPT_POINTLIST: key |= 5ULL << 56; break;
        default: key |= 1ULL << 56; break;
    }
    key |= (uint64_t)(uintptr_t)g->curRp << 40;

    VkPrimitiveTopology topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (pt) {
        case D3DPT_TRIANGLELIST: topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        case D3DPT_TRIANGLESTRIP: topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
        case D3DPT_TRIANGLEFAN:  topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN; break;
        case D3DPT_LINELIST:     topo = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case D3DPT_LINESTRIP:    topo = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
        case D3DPT_POINTLIST:    topo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        default: break;
    }

    VkPipeline pipe = buildPipeline(key, g->curRp, topo);
    if (!pipe) { return; }
    VkDescriptorSet set = allocTexSet(g->bound[0], g->bound[1], (size_t)g->ringCap - uoff);
    if (!set) return;

    vkCmdBindPipeline(g->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    VkDeviceSize bindOff = coff;
    vkCmdBindVertexBuffers(g->cmd, 0, 1, &g->ringBuf, &bindOff);
    uint32_t dyn = (uint32_t)uoff;
    vkCmdBindDescriptorSets(g->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g->pipeLayout, 0, 1, &set, 1, &dyn);
    vkCmdDraw(g->cmd, outCount, 1, 0, 0);
}

static void end_pass()
{ if (g->passOpen) { vkCmdEndRenderPass(g->cmd); g->passOpen = false; } }

/* ---- device COM functions ---- */

static HRESULT dx8dev_QueryInterface(void* self, const void* riid, void** ppv)
{ (void)riid; if (!ppv) return E_INVALIDARG; *ppv = self; InterlockedIncrement(&((Dx8Head*)self)->refs); return S_OK; }
static ULONG dx8dev_AddRef(void* self) { return (ULONG)InterlockedIncrement(&((Dx8Head*)self)->refs); }
static ULONG dx8dev_Release(void* self) { return (ULONG)InterlockedDecrement(&((Dx8Head*)self)->refs); }
static UINT dx8dev_GetAvailableTextureMem(void* self) { (void)self; return 256u << 20; }
static HRESULT dx8dev_GetDirect3D(void* self, void** pp)
{ (void)self; if (!pp) return D3DERR_INVALIDCALL; return D3D_OK; } /* root not exposed; not used by WW3D */
static HRESULT dx8dev_GetDeviceCaps(void* self, void* pCaps)
{
    (void)self;
    D3DCAPS8* c = (D3DCAPS8*)pCaps;
    if (!c) return D3DERR_INVALIDCALL;
    memset(c, 0, sizeof(*c));
    c->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
    c->MaxTextureWidth = c->MaxTextureHeight = 4096;
    c->MaxTextureBlendStages = 2;
    c->MaxSimultaneousTextures = 2;
    c->PresentationIntervals = 1;
    return D3D_OK;
}
static HRESULT dx8dev_GetDisplayMode(void* self, UINT adapter, UINT mode, void* p)
{ (void)self; (void)adapter; (void)mode; D3DDISPLAYMODE* d = (D3DDISPLAYMODE*)p; if (!d) return D3DERR_INVALIDCALL;
  d->Width = g->width; d->Height = g->height; d->RefreshRate = 60; d->Format = D3DFMT_X8R8G8B8; return D3D_OK; }
static HRESULT dx8dev_GetCreationParameters(void* self, void* p)
{ (void)self; D3DDEVICE_CREATION_PARAMETERS* d = (D3DDEVICE_CREATION_PARAMETERS*)p; if (!d) return D3DERR_INVALIDCALL;
  memset(d, 0, sizeof(*d)); d->AdapterOrdinal = 0; d->DeviceType = D3DDEVTYPE_HAL; d->hFocusWindow = g->hwnd; d->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING; return D3D_OK; }
static HRESULT dx8dev_ShowCursor(void* self, int b) { (void)self; return (HRESULT)(DWORD)(UINT)b; }
static HRESULT dx8dev_SetCursorPosition(void* self, int x, int y) { (void)self; (void)x; (void)y; return D3D_OK; }
static HRESULT dx8dev_CreateAdditionalSwapChain(void* self, void* p, void** pp) { (void)self; (void)p; if (pp) *pp = nullptr; return D3DERR_NOTAVAILABLE; }
static HRESULT dx8dev_Reset(void* self, void* ppPresent)
{
    (void)self;
    D3DPRESENT_PARAMETERS* p = (D3DPRESENT_PARAMETERS*)ppPresent;
    vkQueueWaitIdle(g->queue);
    uint32_t w = p && p->BackBufferWidth ? p->BackBufferWidth : g->width;
    uint32_t h = p && p->BackBufferHeight ? p->BackBufferHeight : g->height;
    g->width = w; g->height = h;
    end_pass();
    if (g->cmd) { vkEndCommandBuffer(g->cmd); }
    g->frameStarted = false;
    g->vd.RecreateSwapchain((int)w, (int)h);
    g->swapRP = VK_NULL_HANDLE;
    for (size_t i = 0; i < g->swapFbs.size(); i++) vkDestroyFramebuffer(g->dev, g->swapFbs[i], nullptr);
    g->swapFbs.clear();
    return D3D_OK;
}
static HRESULT dx8dev_Present(void* self, const RECT* s, const RECT* d, HWND win, const void* dirty)
{
    (void)self; (void)s; (void)d; (void)dirty;
    if (win) g->hwnd = win;
    if (!g->frameStarted) return D3D_OK;
    end_pass();
    vkEndCommandBuffer(g->cmd);
    VkSubmitInfo si = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    si.commandBufferCount = 1; si.pCommandBuffers = &g->cmd;
    si.signalSemaphoreCount = 1; si.pSignalSemaphores = &g->presentSem[g->frame];
    bridge_vk_assert(vkQueueSubmit(g->queue, 1, &si, g->fences[g->frame]), "submit");
    vkQueueWaitIdle(g->queue);   /* safe fallback; fence is pre-signalled next frame */

    VkPresentInfoKHR pi = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    pi.swapchainCount = 1; VkSwapchainKHR sc = g->vd.GetSwapchain(); pi.pSwapchains = &sc;
    pi.pImageIndices = &g->acquiredIndex;
    VkResult pr = vkQueuePresentKHR(g->queue, &pi);
    if (pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR) {
        RECT rc; GetClientRect(g->hwnd, &rc);
        D3DPRESENT_PARAMETERS dummy = {}; dummy.BackBufferWidth = rc.right - rc.left; dummy.BackBufferHeight = rc.bottom - rc.top;
        dx8dev_Reset(self, &dummy);
    }
    g->frame = (g->frame + 1) % Bridge::MAX_FRAMES;
    g->frameStarted = false;
    g->acquiredIndex = 0;
    (void)waitStage;
    return D3D_OK;
}
static HRESULT dx8dev_GetBackBuffer(void* self, UINT swap, UINT type, void** pp)
{
    (void)self; (void)swap; (void)type;
    if (!pp) return D3DERR_INVALIDCALL;
    SurfObj* sf = (SurfObj*)calloc(1, sizeof(SurfObj));
    sf->vtbl = vtbl_sf(); sf->refs = 1;
    sf->w = g->width; sf->h = g->height; sf->fmt = D3DFMT_A8R8G8B8;
    *pp = sf;
    return D3D_OK;
}
static HRESULT dx8dev_SetGammaRamp(void* self, UINT which, const void* ramp) { (void)self; (void)which; (void)ramp; return D3D_OK; }
static HRESULT dx8dev_GetGammaRamp(void* self, UINT which, void* ramp) { (void)self; (void)which; (void)ramp; return D3D_OK; }

static BufObj* new_buf(void** vptr, UINT size, D3DFORMAT ifmt)
{
    BufObj* b = (BufObj*)calloc(1, sizeof(BufObj));
    b->vtbl = vptr; b->refs = 1; b->size = size; b->indexFmt = ifmt;
    b->isIndex = (ifmt != (D3DFORMAT)0);
    b->mirror = (uint8_t*)calloc(1, size ? size : 1);
    return b;
}

static HRESULT dx8dev_CreateVertexBuffer(void* self, UINT len, DWORD usage, DWORD fvf, int pool, void** pp)
{ (void)self; (void)usage; (void)fvf; (void)pool; if (!pp) return D3DERR_INVALIDCALL; BufObj* b = new_buf(vtbl_vb(), len, (D3DFORMAT)0); *pp = b; return D3D_OK; }
static HRESULT dx8dev_CreateIndexBuffer(void* self, UINT len, DWORD usage, D3DFORMAT fmt, int pool, void** pp)
{ (void)self; (void)usage; (void)pool; if (!pp) return D3DERR_INVALIDCALL; BufObj* b = new_buf(vtbl_ib(), len, fmt); *pp = b; return D3D_OK; }

static TexObj* new_tex(UINT w, UINT h, UINT levels, D3DFORMAT fmt, DWORD usage)
{
    TexObj* t = (TexObj*)calloc(1, sizeof(TexObj));
    t->vtbl = vtbl_tx(); t->refs = 1;
    t->w = w; t->h = h; t->levels = levels ? levels : 1; t->fmt = fmt;
    VkFormat vkf = vk_format_for(fmt);
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (usage & D3DUSAGE_RENDERTARGET) usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (vkf != VK_FORMAT_UNDEFINED) {
        VkImageCreateInfo ci = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        ci.imageType = VK_IMAGE_TYPE_2D; ci.format = vkf;
        ci.extent = { w ? w : 1, h ? h : 1, 1 };
        ci.mipLevels = 1; ci.arrayLayers = 1; ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.tiling = (usage & D3DUSAGE_RENDERTARGET) ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
        ci.usage = usageFlags;
        if (vkCreateImage(g->dev, &ci, nullptr, &t->img) == VK_SUCCESS) {
            VkMemoryRequirements r; vkGetImageMemoryRequirements(g->dev, t->img, &r);
            VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
            ai.allocationSize = r.size;
            ai.memoryTypeIndex = g->findMemoryType(r.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(g->dev, &ai, nullptr, &t->mem) == VK_SUCCESS)
                vkBindImageMemory(g->dev, t->img, t->mem, 0);
            VkImageViewCreateInfo vci = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            vci.image = t->img; vci.viewType = VK_IMAGE_VIEW_TYPE_2D; vci.format = vkf;
            vci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            vkCreateImageView(g->dev, &vci, nullptr, &t->view);
        } else t->img = VK_NULL_HANDLE;
    }
    return t;
}

static HRESULT dx8dev_CreateTexture(void* self, UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, int pool, void** pp)
{ (void)self; (void)pool; if (!pp) return D3DERR_INVALIDCALL; *pp = new_tex(w, h, levels, fmt, usage); return D3D_OK; }
static HRESULT dx8dev_CreateVolumeTexture(void* self, UINT w, UINT h, UINT d, UINT lv, DWORD u, D3DFORMAT f, int p, void** pp)
{ (void)self; (void)d; (void)u; (void)p; if (!pp) return D3DERR_INVALIDCALL; *pp = new_tex(w, h, lv, f, 0); return D3D_OK; }
static HRESULT dx8dev_CreateCubeTexture(void* self, UINT edge, UINT lv, DWORD u, D3DFORMAT f, int p, void** pp)
{ (void)self; (void)u; (void)p; if (!pp) return D3DERR_INVALIDCALL; *pp = new_tex(edge, edge, lv, f, 0); return D3D_OK; }

static SurfObj* new_rt(UINT w, UINT h, D3DFORMAT fmt, bool depth)
{
    SurfObj* sf = (SurfObj*)calloc(1, sizeof(SurfObj));
    sf->vtbl = vtbl_sf(); sf->refs = 1; sf->w = w; sf->h = h; sf->fmt = fmt; sf->isDepth = depth;
    VkFormat vkf = vk_format_for(fmt);
    if (vkf == VK_FORMAT_UNDEFINED) return sf;
    VkImageCreateInfo ci = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    ci.imageType = VK_IMAGE_TYPE_2D; ci.format = vkf;
    ci.extent = { w, h, 1 }; ci.mipLevels = 1; ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT; ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                     : (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    if (vkCreateImage(g->dev, &ci, nullptr, &sf->img) == VK_SUCCESS) {
        VkMemoryRequirements r; vkGetImageMemoryRequirements(g->dev, sf->img, &r);
        VkMemoryAllocateInfo ai = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = r.size;
        ai.memoryTypeIndex = g->findMemoryType(r.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vkAllocateMemory(g->dev, &ai, nullptr, &sf->mem);
        vkBindImageMemory(g->dev, sf->img, sf->mem, 0);
        VkImageViewCreateInfo vci = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vci.image = sf->img; vci.viewType = VK_IMAGE_VIEW_TYPE_2D; vci.format = vkf;
        vci.subresourceRange = { depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (fmt == D3DFMT_D24S8 || fmt == D3DFMT_D24X4S4) vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        vkCreateImageView(g->dev, &vci, nullptr, &sf->view);
    }
    return sf;
}

static HRESULT dx8dev_CreateRenderTarget(void* self, UINT w, UINT h, D3DFORMAT fmt, int ms, WINBOOL lockable, void** pp)
{ (void)self; (void)ms; (void)lockable; if (!pp) return D3DERR_INVALIDCALL; *pp = new_rt(w, h, fmt, false); return D3D_OK; }
static HRESULT dx8dev_CreateDepthStencilSurface(void* self, UINT w, UINT h, D3DFORMAT fmt, int ms, void** pp)
{ (void)self; (void)ms; if (!pp) return D3DERR_INVALIDCALL; *pp = new_rt(w, h, fmt, true); return D3D_OK; }
static HRESULT dx8dev_CreateImageSurface(void* self, UINT w, UINT h, D3DFORMAT fmt, void** pp)
{ (void)self; if (!pp) return D3DERR_INVALIDCALL;
  SurfObj* sf = (SurfObj*)calloc(1, sizeof(SurfObj)); sf->vtbl = vtbl_sf(); sf->refs = 1;
  sf->w = w; sf->h = h; sf->fmt = fmt; *pp = sf; return D3D_OK; }

static HRESULT dx8dev_CopyRects(void* self, void* dst, const void* dstRects, UINT dstCount, void* src, const void* srcRects, UINT srcCount)
{ (void)self; (void)dst; (void)dstRects; (void)dstCount; (void)src; (void)srcRects; (void)srcCount; return D3D_OK; }
static HRESULT dx8dev_UpdateTexture(void* self, void* s, void* d, const void* sx, const void* sy, UINT flags)
{ (void)self; (void)s; (void)d; (void)sx; (void)sy; (void)flags; return D3DERR_NOTAVAILABLE; }

static HRESULT dx8dev_SetRenderTarget(void* self, void* target, void* z)
{ (void)self; (void)z;
  if (!g->cmd) { ensure_frame_started(); }
  VkFramebuffer fb; VkRenderPass rp; uint32_t w, h;
  rt_for_surface(target, &fb, &rp, &w, &h);
  if (g->passOpen) {
      /* flush any recorded draws for the previous target first */
      end_pass();
  }
  begin_pass(fb, rp, w, h, true);
  return D3D_OK;
}
static HRESULT dx8dev_GetRenderTarget(void* self, void** pp)
{ (void)self; if (!pp) return D3DERR_INVALIDCALL; *pp = nullptr; return D3D_OK; }
static HRESULT dx8dev_GetDepthStencilSurface(void* self, void** pp)
{ (void)self; if (!pp) return D3DERR_INVALIDCALL; *pp = nullptr; return D3D_OK; }

static HRESULT dx8dev_BeginScene(void* self)
{ (void)self;
  ensure_frame_started();
  if (!g->passOpen) {
      VkFramebuffer fb; VkRenderPass rp; uint32_t w, h;
      default_target(&fb, &rp, &w, &h);
      begin_pass(fb, rp, w, h, true);
  }
  return D3D_OK;
}
static HRESULT dx8dev_EndScene(void* self) { (void)self; end_pass(); return D3D_OK; }

static HRESULT dx8dev_Clear(void* self, DWORD count, const D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, unsigned stencil)
{
    (void)self; (void)count; (void)rects;
    if (flags & D3DCLEAR_TARGET) {
        float r = ((color >> 16) & 0xff) / 255.0f;
        float g_ = ((color >> 8) & 0xff) / 255.0f;
        float b = (color & 0xff) / 255.0f;
        g->clearVals[0].color.float32[0] = b;   /* swapped to match begin_pass unpack (see below) */
        g->clearVals[0].color.float32[1] = g_;
        g->clearVals[0].color.float32[2] = r;
        g->clearVals[0].color.float32[3] = 1.0f;
    }
    g->clearVals[1].depthStencil.depth = z;
    g->clearVals[1].depthStencil.stencil = stencil;
    g->clearFlags = flags;
    if (g->passOpen) {
        VkClearAttachment ca[2]; uint32_t n = 0;
        if (flags & D3DCLEAR_TARGET) { ca[n].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; ca[n].colorAttachment = 0; ca[n].clearValue = g->clearVals[0]; n++; }
        if (flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)) { ca[n].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; ca[n].clearValue = g->clearVals[1]; n++; }
        VkClearRect cr = {};
        cr.rect.offset.x = 0; cr.rect.offset.y = 0;
        cr.rect.extent.width = g->curW; cr.rect.extent.height = g->curH;
        cr.baseArrayLayer = 0; cr.layerCount = 1;
        if (n) vkCmdClearAttachments(g->cmd, n, ca, 1, &cr);
    }
    return D3D_OK;
}

static HRESULT dx8dev_SetTransform(void* self, int state, const D3DMATRIX* m)
{ (void)self; unsigned idx = (unsigned)state <= 2u ? (unsigned)state : (state >= 256u && state <= 263u) ? 8u + (unsigned)state - 256u : 0u; if (idx >= 16 || !m) return D3DERR_INVALIDCALL; memcpy(g->transforms[idx], &m->m[0][0], 64); return D3D_OK; }
static HRESULT dx8dev_GetTransform(void* self, int state, void* m)
{ (void)self; unsigned idx = (unsigned)state <= 2u ? (unsigned)state : (state >= 256u && state <= 263u) ? 8u + (unsigned)state - 256u : 0u; if (idx >= 16 || !m) return D3DERR_INVALIDCALL; memcpy(m, g->transforms[idx], 64); return D3D_OK; }
static HRESULT dx8dev_MultiplyTransform(void* self, int d, int s1, int s2)
{ (void)self; (void)s1; (void)s2; if ((unsigned)d >= 16) return D3DERR_INVALIDCALL;
  (void)s1; (void)s2; return D3D_OK; }
static HRESULT dx8dev_SetViewport(void* self, const void* p)
{ (void)self; if (p && g->cmd && g->passOpen) {
    const D3DVIEWPORT8* v = (const D3DVIEWPORT8*)p;
    VkViewport vp = { (float)v->X, (float)v->Y, (float)v->Width, (float)v->Height, v->MinZ, v->MaxZ };
    if (vp.height < 0) { vp.y += vp.height; vp.height = -vp.height; }
    vkCmdSetViewport(g->cmd, 0, 1, &vp);
  } return D3D_OK; }
static HRESULT dx8dev_GetViewport(void* self, void* p) { (void)self; (void)p; return D3D_OK; }
static HRESULT dx8dev_SetMaterial(void* self, const void* m) { (void)self; if (m) g->material = *(const D3DMATERIAL8*)m; return D3D_OK; }
static HRESULT dx8dev_GetMaterial(void* self, void* m) { (void)self; if (m) *(D3DMATERIAL8*)m = g->material; return D3D_OK; }
static HRESULT dx8dev_SetLight(void* self, DWORD i, const void* l)
{ (void)self; if (i < 8 && l) g->lights[i] = *(const D3DLIGHT8*)l; return D3D_OK; }
static HRESULT dx8dev_GetLight(void* self, DWORD i, void* l)
{ (void)self; if (i < 8 && l) *(D3DLIGHT8*)l = g->lights[i]; return D3D_OK; }
static HRESULT dx8dev_LightEnable(void* self, DWORD i, WINBOOL e)
{ (void)self; if (i < 8) g->lightEnable[i] = e; return D3D_OK; }
static HRESULT dx8dev_GetLightEnable(void* self, DWORD i, void* pe)
{ (void)self; if (i < 8 && pe) *(WINBOOL*)pe = g->lightEnable[i] ? TRUE : FALSE; return D3D_OK; }
static HRESULT dx8dev_SetRenderState(void* self, DWORD st, DWORD v)
{ (void)self; if (st < 512) {
    g->rs[st] = v;

  } return D3D_OK; }
static HRESULT dx8dev_GetRenderState(void* self, DWORD st, DWORD* pv)
{ (void)self; if (pv) *pv = (st < 512) ? g->rs[st] : 0; return D3D_OK; }
static HRESULT dx8dev_SetTexture(void* self, DWORD stage, void* tex)
{ (void)self; if (stage < 4) g->bound[stage] = (TexObj*)tex; return D3D_OK; }
static HRESULT dx8dev_GetTexture(void* self, DWORD stage, void** pp)
{ (void)self; if (stage < 4 && pp) *pp = g->bound[stage]; return D3D_OK; }
static HRESULT dx8dev_SetTextureStageState(void* self, DWORD stage, DWORD type, DWORD v)
{ (void)self; if (stage >= 4) return D3D_OK;
  Bridge::Stage* st = &g->stages[stage];
  switch (type) {
    case D3DTSS_COLOROP: st->colorOp = v; st->op = 1; break;
    case D3DTSS_COLORARG1: st->arg1 = v; break;
    case D3DTSS_COLORARG2: st->arg2 = v; break;
    case D3DTSS_ALPHAOP: st->alphaOp = v; break;
    case D3DTSS_ALPHAARG1: break;
    case D3DTSS_ALPHAARG2: break;
    case D3DTSS_TEXCOORDINDEX: st->uv = (v & 3); break;
    default: break;
  }
  return D3D_OK; }
static HRESULT dx8dev_GetTextureStageState(void* self, DWORD stage, DWORD type, DWORD* pv)
{ (void)self; (void)stage; (void)type; if (pv) *pv = 0; return D3D_OK; }
static HRESULT dx8dev_ValidateDevice(void* self, DWORD* pNumPasses)
{ (void)self; if (pNumPasses) *pNumPasses = 1; return D3D_OK; }
static HRESULT dx8dev_SetStreamSource(void* self, UINT stream, void* vb, UINT stride)
{ (void)self; (void)stream; g->stream = (BufObj*)vb; g->streamStride = stride; return D3D_OK; }
static HRESULT dx8dev_GetStreamSource(void* self, UINT stream, void** pp, UINT* pStride)
{ (void)self; (void)stream; if (pp) *pp = g->stream; if (pStride) *pStride = g->streamStride; return D3D_OK; }
static HRESULT dx8dev_SetIndices(void* self, void* ib, UINT baseIdx)
{ (void)self; (void)baseIdx; g->ibuf = (BufObj*)ib; return D3D_OK; }
static HRESULT dx8dev_GetIndices(void* self, void** pp, UINT* pBase)
{ (void)self; if (pp) *pp = g->ibuf; if (pBase) *pBase = 0; return D3D_OK; }
static HRESULT dx8dev_SetVertexShader(void* self, DWORD h)
{ (void)self;
  if (h & 0xFFFF0000u) dx8vk_log_once("vsh", "shader-handle vertex shader %08x ignored (using last FVF)", h);
  else g->fvf = h;
  return D3D_OK; }
static HRESULT dx8dev_GetVertexShader(void* self, DWORD* ph) { (void)self; if (ph) *ph = g->fvf; return D3D_OK; }
static HRESULT dx8dev_DeleteVertexShader(void* self, DWORD h) { (void)self; (void)h; return D3D_OK; }
static HRESULT dx8dev_SetVertexShaderConstant(void* self, DWORD reg, const void* data, DWORD n)
{ (void)self; (void)reg; (void)data; (void)n; return D3D_OK; }
static HRESULT dx8dev_CreateVertexShader(void* self, const void* decl, const void* fsize, DWORD* ph, DWORD usage)
{ (void)self; (void)decl; (void)fsize; (void)usage; (void)ph; return D3DERR_NOTAVAILABLE; }
static HRESULT dx8dev_CreatePixelShader(void* self, const void* code, DWORD* ph)
{ (void)self; (void)code; if (ph) *ph = 0; return D3DERR_NOTAVAILABLE; }
static HRESULT dx8dev_SetPixelShader(void* self, DWORD h) { (void)self; (void)h; return D3D_OK; }
static HRESULT dx8dev_GetPixelShader(void* self, DWORD* ph) { (void)self; if (ph) *ph = 0; return D3D_OK; }
static HRESULT dx8dev_DeletePixelShader(void* self, DWORD h) { (void)self; (void)h; return D3D_OK; }
static HRESULT dx8dev_SetPixelShaderConstant(void* self, DWORD reg, const void* data, DWORD n)
{ (void)self; (void)reg; (void)data; (void)n; return D3D_OK; }
static HRESULT dx8dev_GetPixelShaderConstant(void* self, DWORD reg, void* data, DWORD n)
{ (void)self; (void)reg; if (data && n) memset(data, 0, n * 16); return D3D_OK; }
static HRESULT dx8dev_ProcessVertices(void* self, UINT sp, UINT count, void* out, void* shader, DWORD unused)
{ (void)self; (void)sp; (void)count; (void)out; (void)shader; (void)unused;
  dx8vk_log_once("procvert", "ProcessVertices unsupported by Vulkan bridge");
  return D3DERR_INVALIDCALL; }
static HRESULT dx8dev_GetInfo(void* self, DWORD cmd, void* p, DWORD* psz) { (void)self; (void)cmd; (void)p; if (psz) *psz = 0; return D3DERR_INVALIDCALL; }
static HRESULT dx8dev_ResourceManagerDiscardBytes(void* self, DWORD bytes) { (void)self; (void)bytes; return D3D_OK; }

static HRESULT dx8dev_DrawPrimitive(void* self, D3DPRIMITIVETYPE pt, UINT start, UINT count)
{
    (void)self;
    if (!g->passOpen) { VkFramebuffer fb; VkRenderPass rp; uint32_t w, h; default_target(&fb, &rp, &w, &h); begin_pass(fb, rp, w, h, false); }
    emit_draw(pt, count, nullptr, nullptr, 0, 0, start, 0);
    return D3D_OK;
}
static HRESULT dx8dev_DrawIndexedPrimitive(void* self, D3DPRIMITIVETYPE pt, UINT minV, UINT numV, UINT startV, UINT primCount)
{
    (void)self; (void)minV; (void)numV;
    if (!g->passOpen) { VkFramebuffer fb; VkRenderPass rp; uint32_t w, h; default_target(&fb, &rp, &w, &h); begin_pass(fb, rp, w, h, false); }
    if (!g->ibuf) return D3DERR_INVALIDCALL;
    uint32_t idxCount = primCount;
    switch (pt) {
        case D3DPT_TRIANGLELIST: idxCount = 3 * primCount; break;
        case D3DPT_TRIANGLESTRIP: idxCount = primCount + 2; break;
        case D3DPT_TRIANGLEFAN: idxCount = primCount + 2; break;
        case D3DPT_LINELIST: idxCount = 2 * primCount; break;
        case D3DPT_LINESTRIP: idxCount = primCount + 1; break;
        default: break;
    }
    emit_draw(pt, primCount, nullptr, g->ibuf, startV, idxCount, minV, 0);
    return D3D_OK;
}
static HRESULT dx8dev_DrawPrimitiveUP(void* self, D3DPRIMITIVETYPE pt, UINT primCount, const void* data, UINT vertexStrideByteCount)
{
    (void)self;
    if (!g->passOpen) { VkFramebuffer fb; VkRenderPass rp; uint32_t w, h; default_target(&fb, &rp, &w, &h); begin_pass(fb, rp, w, h, false); }
    /* temporary stream swap */
    BufObj tmp; memset(&tmp, 0, sizeof(tmp));
    tmp.mirror = (uint8_t*)data; tmp.size = vertexStrideByteCount * 65535; tmp.dirty = false;
    BufObj* saved = g->stream; UINT savedStride = g->streamStride;
    g->stream = &tmp; g->streamStride = vertexStrideByteCount;
    emit_draw(pt, primCount, nullptr, nullptr, 0, 0, 0, 0);
    g->stream = saved; g->streamStride = savedStride;
    return D3D_OK;
}
static HRESULT dx8dev_DrawIndexedPrimitiveUP(void* self, D3DPRIMITIVETYPE pt, UINT minV, UINT numV, UINT primCount, const void* idxData, D3DFORMAT idxFmt, UINT vertexStrideByteCount)
{
    (void)self; (void)numV;
    if (!g->passOpen) { VkFramebuffer fb; VkRenderPass rp; uint32_t w, h; default_target(&fb, &rp, &w, &h); begin_pass(fb, rp, w, h, false); }
    BufObj tmp; memset(&tmp, 0, sizeof(tmp));
    tmp.mirror = (uint8_t*)idxData; tmp.size = 65535 * 4; tmp.isIndex = true; tmp.indexFmt = idxFmt;
    BufObj* savedS = g->stream; BufObj* savedI = g->ibuf;
    BufObj vtmp; memset(&vtmp, 0, sizeof(vtmp));
    /* need a stream too: caller sets stream via SetStreamSource normally; if not set, fail */
    if (!g->stream) { g->stream = nullptr; return D3DERR_INVALIDCALL; }
    g->ibuf = &tmp;
    uint32_t idxCount = primCount;
    switch (pt) {
        case D3DPT_TRIANGLELIST: idxCount = 3 * primCount; break;
        case D3DPT_TRIANGLESTRIP: idxCount = primCount + 2; break;
        case D3DPT_LINELIST: idxCount = 2 * primCount; break;
        case D3DPT_LINESTRIP: idxCount = primCount + 1; break;
        default: break;
    }
    emit_draw(pt, primCount, nullptr, &tmp, 0, idxCount, minV, 0);
    g->ibuf = savedI; (void)vtmp;
    return D3D_OK;
}

/* remaining declared-impl device stubs (kept here so the vtable references exist) */

static HRESULT dx8tx_GetSurfaceLevel(void* self, UINT level, void** ppSurface)
{
    TexObj* t = (TexObj*)self;
    if (!ppSurface) return D3DERR_INVALIDCALL;
    if (level == 0) {
        if (!t->level0Surf) {
            SurfObj* sf = (SurfObj*)calloc(1, sizeof(SurfObj));
            sf->vtbl = vtbl_sf(); sf->refs = 1; sf->parentTex = t; sf->level = 0;
            sf->w = t->w; sf->h = t->h; sf->fmt = t->fmt;
            t->level0Surf = sf;
        } else {
            InterlockedIncrement(&((Dx8Head*)t->level0Surf)->refs);
        }
        *ppSurface = t->level0Surf;
        return D3D_OK;
    }
    SurfObj* sf = (SurfObj*)calloc(1, sizeof(SurfObj));
    sf->vtbl = vtbl_sf(); sf->refs = 1; sf->parentTex = t; sf->level = level;
    UINT d = level < t->levels ? level : (t->levels ? t->levels - 1 : 0);
    sf->w = (t->w >> d) ? (t->w >> d) : 1;
    sf->h = (t->h >> d) ? (t->h >> d) : 1;
    sf->fmt = t->fmt;
    *ppSurface = sf;
    return D3D_OK;
}

/* ------------------------------------------------------------------ */
/* root IDirect3D8                                                     */
/* ------------------------------------------------------------------ */

struct RootObj { void** vtbl; volatile LONG refs; };

static UINT dx8rb_GetAdapterCount(void* self) { (void)self; return 1; }
static HRESULT dx8rb_GetAdapterIdentifier(void* self, UINT a, DWORD flags, void* pIdent)
{
    (void)self; (void)a; (void)flags;
    D3DADAPTER_IDENTIFIER8* id = (D3DADAPTER_IDENTIFIER8*)pIdent;
    if (!id) return D3DERR_INVALIDCALL;
    memset(id, 0, sizeof(*id));
    strcpy(id->Driver, "dx8vk");
    strcpy(id->Description, "WW3D Vulkan device bridge");
    id->WHQLLevel = 1;
    return D3D_OK;
}
static UINT dx8rb_GetAdapterModeCount(void* self, UINT a) { (void)self; (void)a; return 1; }
static HRESULT dx8rb_EnumAdapterModes(void* self, UINT a, UINT m, void* p)
{ (void)self; (void)a; (void)m; D3DDISPLAYMODE* d = (D3DDISPLAYMODE*)p; if (!d) return D3DERR_INVALIDCALL;
  LONG w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
  if (g && g->hwnd) { RECT rc; if (GetClientRect(g->hwnd, &rc) && rc.right > 0 && rc.bottom > 0) { w = rc.right; h = rc.bottom; } }
  if (w <= 0) w = 1280; if (h <= 0) h = 720;
  d->Width = (UINT)w; d->Height = (UINT)h; d->RefreshRate = 60; d->Format = D3DFMT_X8R8G8B8; return D3D_OK; }
static HRESULT dx8rb_GetAdapterDisplayMode(void* self, UINT a, void* p) { return dx8rb_EnumAdapterModes(self, a, 0, p); }
static HRESULT dx8rb_CheckDeviceType(void* self, UINT a, int devType, D3DFORMAT display, D3DFORMAT back, WINBOOL windowed)
{ (void)self; (void)a; (void)devType; (void)display; (void)back; (void)windowed; return D3D_OK; }
static HRESULT dx8rb_CheckDeviceFormat(void* self, UINT a, int devType, D3DFORMAT fmt, DWORD usage, int rtype, D3DFORMAT c)
{ (void)self; (void)a; (void)devType; (void)fmt; (void)usage; (void)rtype; (void)c; return D3D_OK; }
static HRESULT dx8rb_CheckDeviceMultiSampleType(void* self, UINT a, int devType, D3DFORMAT fmt, WINBOOL windowed, int ms)
{ (void)self; (void)a; (void)devType; (void)fmt; (void)windowed; (void)ms; return D3D_OK; }
static HRESULT dx8rb_CheckDepthStencilMatch(void* self, UINT a, int devType, D3DFORMAT adapter, D3DFORMAT depth, D3DFORMAT fmt)
{ (void)self; (void)a; (void)adapter; (void)depth; (void)fmt; return D3D_OK; }
static HRESULT dx8rb_GetDeviceCaps(void* self, UINT a, int type, void* pCaps)
{
    (void)self; (void)a; (void)type;
    D3DCAPS8* c = (D3DCAPS8*)pCaps;
    if (!c) return D3DERR_INVALIDCALL;
    memset(c, 0, sizeof(*c));
    c->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
    c->MaxTextureWidth = c->MaxTextureHeight = 4096;
    c->MaxTextureBlendStages = c->MaxSimultaneousTextures = 2;
    c->MaxActiveLights = 1;
    c->PrimitiveMiscCaps = 0;
    return D3D_OK;
}
static HRESULT dx8rb_CreateDevice(void* self, UINT adapter, int devType, HWND hwnd, DWORD flags, void* ppPresent, void** ppDevice)
{
    (void)adapter; (void)devType; (void)flags;
    if (!ppDevice) return D3DERR_INVALIDCALL;
    D3DPRESENT_PARAMETERS* p = (D3DPRESENT_PARAMETERS*)ppPresent;
    uint32_t w = p && p->BackBufferWidth ? p->BackBufferWidth : 1280;
    uint32_t h = p && p->BackBufferHeight ? p->BackBufferHeight : 720;
    if (!hwnd) { dx8vk_log("CreateDevice without HWND"); return D3DERR_INVALIDCALL; }
    if (!bridge_init(hwnd, w, h)) return D3DERR_INVALIDDEVICE;
    DevObj* d = (DevObj*)calloc(1, sizeof(DevObj));
    d->vtbl = vtbl_dev(); d->refs = 1;
    g->deviceObj = d;
    *ppDevice = d;
    dx8vk_log("Vulkan bridge device created (%ux%u hwnd=%p)", w, h, (void*)hwnd);
    return D3D_OK;
}

/* ------------------------------------------------------------------ */
/* generated vtables + accessors                                       */
/* ------------------------------------------------------------------ */

#define DX8_STUB(nm) static unsigned long long nm(void* self) { (void)self; return 0; }

#include "dx8vk_vtbl_gen.inc"

#undef DX8_STUB

static void** vtbl_root(void) { return (void**)dx8vt_dx8rb; }
static void** vtbl_dev(void)  { return (void**)dx8vt_dx8dev; }
static void** vtbl_vb(void)   { return (void**)dx8vt_dx8vb; }
static void** vtbl_ib(void)   { return (void**)dx8vt_dx8ib; }
static void** vtbl_tx(void)   { return (void**)dx8vt_dx8tx; }
static void** vtbl_sf(void)   { return (void**)dx8vt_dx8sf; }

/* ------------------------------------------------------------------ */
/* public entry                                                        */
/* ------------------------------------------------------------------ */

IDirect3D8* DX8Vk_CreateD3D8(void)
{
    RootObj* r = (RootObj*)calloc(1, sizeof(RootObj));
    r->vtbl = vtbl_root();
    r->refs = 1;
    return (IDirect3D8*)r;
}

bool DX8Vk_ShouldUseBridge(void)
{
    const char* env = getenv("CNC_VULKAN");
    return env && env[0] && strcmp(env, "0") != 0;
}
