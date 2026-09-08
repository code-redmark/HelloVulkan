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

struct VkMeshData {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct VkGpuMesh
{
    VkBuffer buffer;
    VmaAllocation allocation;
    
    VkDeviceSize vertex_offset;
    VkDeviceSize index_offset;

    uint32_t index_count;

    void destroyBuffer(VmaAllocator vma);
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


