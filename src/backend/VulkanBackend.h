#pragma once

/*
	needed to get the VK_KHR_win32_Surface extension, in GSAM we're going to 
	select the right one based on the user's OS, im on Windows so WIN32
*/
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
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

#define GSAM_VK_CHECK(result, err_msg) if (result != VK_SUCCESS) GSAM_THROW_ERROR(err_msg);

namespace VulkanBackend
{
    class Context;
    class Swapchain;
    class CommandManager;
    class SyncManager;
    class Cleaner;

    struct Buffer
    {
        VkBuffer buffer{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocationInfo{};

        Buffer() = default;

        void Free(VmaAllocator allocator);
    };

    class TextureImplementation : public GSAM::TextureImplementation
    {
        ktxTexture texture;
        VkSampler sampler{};
    };

    struct MeshData {
        std::vector<GSAM::Vertex> vertices;
        std::vector<uint16_t> indices;
    };

    
    struct GpuMesh
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation{};
        
        VkDeviceSize vertex_offset{};
        VkDeviceSize index_offset{};
        
        uint32_t index_count{};
        
        void Free(VmaAllocator vma);
    };
    
    struct VulkanMeshImplementation : public GSAM::MeshImplementation
    {
        TypedResourceHandle<GpuMesh> handle;
    };
    
    struct ShaderData {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model[3];
        glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
        uint32_t selected{1};
    };

    struct ShaderBuffer : public Buffer
    {
        VkDeviceAddress deviceAddress = 0;

        ShaderBuffer(VmaAllocator allocator, const VkDevice& device);
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



    MeshData LoadMesh_Obj(const std::filesystem::path& path);

    struct ImageData
    {
        int width{};
        int height{};
        int channels{};
        stbi_uc* pixels{};
    };

    ImageData load_image(const std::filesystem::path& path);
    struct GpuImage
    {
        VkImage image{};
        VmaAllocation allocation{};
        VkImageView imageView{};
    };
    struct ImageTexture
    {
        GpuImage gpuImage;
        TextureImplementation texture;
    };
}



