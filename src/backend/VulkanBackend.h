#pragma once

#define VK_USE_PLATFORM_XLIB_KHR
#include <wayland-client.h>
#include <vulkan/vulkan.h>

#include "GpuResources.hpp"

#include "deps/vma.h"
#include "deps/srcs/GSAMStuff/GSAM.hpp"

#include "deps/srcs/stb_image/stb_image.h"
#include <ktx.h>
#include <ktxvulkan.h>
#include "deps/srcs/tinyobj/tiny_obj_loader.h"
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <utility>
#include <memory>
#include <set>
#include <fstream>
#include <filesystem>
#include <string>

namespace VulkanBackend
{
    class Context;
    class Swapchain;
    class CommandManager;
    class SyncManager;
    class Cleaner;

    class GpuBuffer;
    class ShaderBuffer;
    class GpuMesh;
    class GpuImage;

    struct MeshData {
        std::vector<GSAM::Vertex> vertices;
        std::vector<uint16_t> indices;
    };

    struct VulkanMeshImplementation : public GSAM::MeshImplementation
    {
        TypedResourceHandle<GpuMesh> handle;
    };
    
    struct ShaderData {
        glm::mat4 projection{};
        glm::mat4 view{};
        glm::mat4 model[3];
        glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
        uint32_t selected{1};
    };

    struct Frame
    {
        uint32_t index{};

        VkCommandBuffer commandBuffer{};
        ShaderBuffer shaderBuffer;

        VkFence fence{};
        VkSemaphore semaphore{};

        void Free(VmaAllocator vma);
    };

    struct DeviceCapabilities
    {
        bool samplerAnisotropySupported = false;
        bool samplerAnisotropyEnabled = false;

        float maxSamplerAnisotropy = 1.0f;
        float maxSamplerLodBias = 0.0f;
    };

    MeshData LoadMesh_Obj(const std::filesystem::path& path);

    struct ImageData
    {
        int width{};
        int height{};
        int channels{};
        stbi_uc* pixels{};
    };

    ImageData load_image(const std::filesystem::path& path);

    struct TextureImplementation : public GSAM::TextureImplementation
    {
        TypedResourceHandle<GpuImage> handle;
        ktxTexture* ktx;
        VkSampler sampler{};
    };
    struct SamplerSettings
    {
        VkFilter magFilter{VK_FILTER_LINEAR};
        VkFilter minFilter{VK_FILTER_LINEAR};
        VkSamplerMipmapMode mipmapMode{VK_SAMPLER_MIPMAP_MODE_LINEAR};
        VkSamplerAddressMode addressModeU{VK_SAMPLER_ADDRESS_MODE_REPEAT};
        VkSamplerAddressMode addressModeV{VK_SAMPLER_ADDRESS_MODE_REPEAT};
        VkSamplerAddressMode addressModeW{VK_SAMPLER_ADDRESS_MODE_REPEAT};
        float mipLodBias{0.0f};
        VkBool32 anisotropyEnable{VK_TRUE};
        float maxAnisotropy{16.0f};
        VkBool32 compareEnable{VK_FALSE};
        VkCompareOp compareOp{VK_COMPARE_OP_ALWAYS};
        float minLod{0.0f};
        float maxLod{1.0f};
        VkBorderColor borderColor{VK_BORDER_COLOR_INT_OPAQUE_BLACK};
        VkBool32 unnormalizedCoordinates{VK_FALSE};
    };

    
    VkSampler create_texture_sampler(const VkDevice& device, const DeviceCapabilities& deviceCaps, const SamplerSettings& settings = SamplerSettings());

}



