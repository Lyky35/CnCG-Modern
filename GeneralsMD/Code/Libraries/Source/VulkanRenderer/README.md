# Vulkan Renderer

This module replaces the legacy Direct3D 8 renderer with a modern Vulkan 1.3 renderer.

## Components

- `VulkanDevice` — Vulkan instance, device, swapchain, synchronization
- `VulkanBuffer` — Vertex/index buffer management
- `VulkanTexture` — Texture/image management
- `VulkanShader` — SPIR-V shader module management
- `VulkanPipeline` — Graphics pipeline state objects
- `VulkanRenderer` — Main rendering interface (replaces DX8Wrapper)
- `Shaders/` — GLSL shader sources

## Requirements

- Vulkan 1.3+ capable GPU
- Vulkan SDK (for validation layers in debug builds)
