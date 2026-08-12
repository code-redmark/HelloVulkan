#pragma once

#include "VulkanBackend.h"

#include "VulkanSwapchain.h"

#include <vector>
#include <array>
#include <optional>
#include <utility>
#include <memory>

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
    std::array<std::optional<int>, capability_count()> queue_families_indices;

    std::unique_ptr<VulkanSwapchain> swapchain = nullptr;
    
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
    bool create_device(FamilyQueueRequirements& requirements);

    bool create_swapchain();

    void shutdown();

    
};