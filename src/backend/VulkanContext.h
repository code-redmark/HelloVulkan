#pragma once

#include "VulkanBackend.h"

#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <utility>
#include <memory>
#include <set>

class VulkanCommandManager;
struct VulkanSwapchain;


class VulkanContext
{

friend class VulkanSwapchain;

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

    VmaAllocator vma;

    std::array<std::optional<int>, capability_count()> queue_families_indices;
    std::unique_ptr<VulkanSwapchain> swapchain = nullptr;
    
    #ifndef NDEBUG
    bool check_validation_layers_support();
    bool create_debug_messenger();
    #endif

    /*
        Creates a VkInstance for the user's platform and
        its specific extensions
    */
    void create_instance();

    /*
        Finds a physical GPU device to run the application,
        returns true if it was found
    */
    void pick_device();

    /*
        Creates a surface to make Vulkan communicate with our
        window manager
    */
    void create_surface(void* win_handle);

    /*
        Creates a "logical" device (VkDevice) through
        the context's physical_device
    */
    void create_device(ApplicationRequirements& requirements);
    
    /*
        Creates a VMA Allocator
    */
    void setup_vma();
    
public:
    VulkanContext(void* window_handle, ApplicationRequirements &requirements);
    void shutdown();
    
};


/*
    Contains all the commands in their command buffers in the
    respective command pools for the given context
*/
class VulkanCommandManager
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


    VulkanCommandManager(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device);


};

class VulkanSwapchain
{

private:

/*
    Only used at destruction time to free the depth image and its memory
*/
VmaAllocator& vma;

// small functions to keep code away from constructor
void createSwapchainKHR();

const std::vector<VkImage>& get_images(); 

void set_queue_families(const std::array<std::optional<int>, capability_count()>& queue_families_indices);

void set_image_format();

/*
    Queries surface capabilities and gets all the data and information
    the swapchain needs to get from it
*/
void set_surface_capability_info(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

void select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/*
    Creates image views for each of the swapchain's images
*/
void create_image_views();

VkFormat get_depth_format(VkPhysicalDevice physical_device);
/*
    Creates a depth image and image view
*/
void create_depth_attachment(VmaAllocator allocator);



public:

VkDevice device;

VkSwapchainKHR swapchain;
VkSwapchainCreateInfoKHR info;

VkSharingMode sharing_mode;
VkSurfaceFormatKHR image_format;
VkColorSpaceKHR image_color_space;

std::vector<VkImage> images;
std::vector<VkImageView> image_views;

VkFormat depth_format;

VkImage depth_image;
VmaAllocation depth_image_allocation;

VkImageView depth_image_view;

VulkanSwapchain(VulkanContext& context);
~VulkanSwapchain();



}; 
