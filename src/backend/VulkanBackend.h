#pragma once

/*
	needed to get the VK_KHR_win32_Surface extension, in GSAM we're going to 
	select the right one based on the user's OS, im on Windows so WIN32
*/
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "deps/srcs/GSAMStuff/GSAMStuff.hpp"
#include "deps/vma.h"
#include <glm/glm.hpp>

#include "deps/srcs/tinyobj/tiny_obj_loader.h"

#include <array>
#include <utility>

#define GSAM_VK_CHECK(result, err_msg) if (result != VK_SUCCESS) GSAM_THROW_ERROR(err_msg);

struct VulkanBuffer
{
    VkBuffer buffer{};
    VmaAllocation allocation{};
    VmaAllocationInfo allocationInfo{};

    VulkanBuffer() = default;
    VulkanBuffer(VmaAllocator allocator, VkBufferCreateInfo bufferCreateInfo, VmaAllocationCreateInfo bufferAllocInfo);
};

struct VulkanMeshData {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct VulkanGpuMesh
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation{};
    
    VkDeviceSize vertex_offset{};
    VkDeviceSize index_offset{};

    uint32_t index_count{};

    void destroyBuffer(VmaAllocator vma);
};

struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model[3];
    glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
    uint32_t selected{1};
};

struct ShaderDataBuffer
{
    VulkanBuffer buffer{};
    VkDeviceAddress deviceAddress = 0;

    ShaderDataBuffer(VmaAllocator allocator, const VkDevice& device);
};

struct VulkanFrame
{
    uint32_t index{};

    VkCommandBuffer commandBuffer{};
    ShaderDataBuffer shaderBuffer;

    VkFence fence{};
    VkSemaphore semaphore{};

    void Free(VmaAllocator vma);
};

enum class FamilyCapability
{
    Graphics = 0,
    Presentation,
    
    Count
};

constexpr inline size_t capability_count() { return static_cast<size_t>(FamilyCapability::Count); }

class ApplicationRequirements
{
private:
    // requires, amount of queues required
   std::array<std::pair<bool, int>, capability_count()> requirements;

public:
    bool requires(FamilyCapability capability) const;
    int queue_requirement(FamilyCapability capability) const;
    void set_requirement(FamilyCapability capability, bool value, int queue_requirement);
};

template <typename E>
constexpr inline int enum_index(E Enum) { return static_cast<int>(Enum); }

template <typename E>
constexpr inline E index_enum(int index) { return static_cast<E>(index); }


