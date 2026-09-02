#pragma once

#include "VulkanBackend.h"

#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <utility>
#include <memory>
#include <set>

#define VAL_LAYERS_NAME "VK_LAYER_KHRONOS_validation"

class CommandManager;
struct VulkanSwapchain;


class VulkanContext
{
private:

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

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
    
    #ifndef NDEBUG
    bool check_validation_layers_support();
    #endif

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
    bool create_device(ApplicationRequirements& requirements);
    
    
public:
    VulkanContext(void* window_handle, ApplicationRequirements &requirements);
    void shutdown();
    
friend class VulkanSwapchain;
};


/*
    Contains all the commands in their command buffers in the
    respective command pools for the given context
*/
class CommandManager
{

friend class VulkanContext;
    
private:

    /*
        A command pool is created for every queue family our 
        context picked up
    */
    std::array<std::optional<VkCommandPool>, capability_count()> pools;
    
    std::vector<VkCommandBuffer> commandBuffers;

    /*
        Creates a command pool for each of the context's queue
        families
    */
    void create_command_pools(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device);


    CommandManager(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device);


};

struct VulkanSwapchain
{

VkDevice device;

VkSwapchainKHR swapchain;

VkSharingMode sharing_mode;
VkFormat image_format;
VkColorSpaceKHR image_color_space;

std::vector<VkImage> images;
std::vector<VkImageView> image_views;

VulkanSwapchain(VulkanContext& context);
~VulkanSwapchain();

}; 
