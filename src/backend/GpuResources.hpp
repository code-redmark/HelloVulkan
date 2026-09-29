#pragma once

#include <vulkan/vulkan.h>

#include "deps/vma.h"
#include "deps/srcs/GSAMStuff/Log.hpp"

namespace VulkanBackend
{
    struct GpuBuffer
    {
        GpuBuffer() = default;

        GpuBuffer(
            VmaAllocator& vma,
            const VkBufferCreateInfo& buffer_create_info,
            const VmaAllocationCreateInfo& allocation_create_info
        );

        VkBuffer buffer{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocationInfo{};

        void Free(const VmaAllocator& vma);

        ~GpuBuffer() = default;
    };

    // its basically a GpuBuffer with specific CreateInfo 
    struct ShaderBuffer : public GpuBuffer
    {
        ShaderBuffer() = default;
        ShaderBuffer(const VmaAllocator& vma, const VkDevice& device);

        VkDeviceAddress deviceAddress{};
    };


    // GpuMesh is an higher level object containing buffers and images
    // which are objects that are totally handled by the backend using
    // the corresponding registries
    struct GpuMesh
    {
        GpuMesh() = default;
        explicit GpuMesh(const GpuBuffer& buffer);

        GpuBuffer buffer{};
    };

    struct GpuImage
    {
        GpuImage() = default;

        GpuImage(
            const VmaAllocator& vma,
            const VkImageCreateInfo& image_create_info,
            const VmaAllocationCreateInfo& allocation_create_info,
            const VkDevice& device,
            VkImageViewCreateInfo& image_view_create_info
        );

        VkImage image{};
        VmaAllocation allocation{};
        VkImageView imageView{};
    };

}