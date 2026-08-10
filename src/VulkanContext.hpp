#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


#include <vector>
#include <array>
#include <optional>
#include <utility>

enum class FamilyCapability
{
    Graphics = 0,
    Presentation,

    Count = 2
};

template <typename E>
constexpr inline int enum_index(E Enum) { return static_cast<int>(Enum); }

template <typename E>
constexpr inline E index_enum(int index) { return static_cast<E>(index); }

constexpr inline size_t capability_count() { return static_cast<size_t>(FamilyCapability::Count); }

class FamilyQueueRequirements
{
private:
    // requires, queue requirement
   std::array<std::pair<bool, int>, capability_count()> requirements;

public:
    bool requires(FamilyCapability capability) const;
    int queue_requirement(FamilyCapability capability) const;
    void set_requirement(FamilyCapability capability, bool value, int queue_requirement);
};

struct VulkanContext
{
    /*
        Represents the vulkan instance
    */
    VkInstance instance = VK_NULL_HANDLE;
    /*
        Represents the physical GPU we're going to use
        in our program
    */
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    /*
        Represents a communication created between our
        VkInstance and our window manager
    */
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    /*
        The logical device used to interact with our
        physical device
    */
    VkDevice device = VK_NULL_HANDLE;
    
    VulkanContext();

    void Init(void* window_handle, FamilyQueueRequirements &requirements);


    /*
        Finds a physical GPU device to run the application,
        returns true if it was found
    */
    bool pick_device();

    /*
        Creates a surface to make Vulkan communicate with our
        window manager
    */
    bool create_surface(void* win_handle);


    /*
        Creates a "logical" device (VkDevice) through
        the context's physical_device
    */
    bool VulkanContext::create_device(FamilyQueueRequirements& requirements);

    void shutdown();

    
};