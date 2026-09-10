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
class VulkanSyncManager;

struct VulkanSwapchain;


class VulkanContext
{

friend class VulkanSwapchain;

private:

    const int MAX_FRAMES_IN_FLIGHT = 2;

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

    /*
        Creates [MAX_FRAMES_IN_FLIGHT] VulkanFrames, assumes the instance's commandManager and syncManager
        have already been initialized and will be produce errors if called before
    */
    void create_frames();

    ResourceRegistry meshRegistry;
    std::vector<ResourceHandle> meshHandles;

    // VkFrame data and functions

    std::unique_ptr<VulkanCommandManager> commandManager;
    std::unique_ptr<VulkanSyncManager> syncManager;

    std::vector<VulkanFrame> frames;
    std::optional<VulkanFrame> CreateFrame();


    VulkanMeshData LoadMesh_Obj(std::string path);
    VulkanGpuMesh UploadMesh(const VulkanMeshData& data);
    
public:
    VulkanContext(void* window_handle, ApplicationRequirements &requirements);
    void shutdown();

    ResourceHandle CreateMesh(std::string path);

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
    
public:

VulkanCommandManager(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device, const int max_frames_in_flight);
void Free(const VkDevice& device);

VkCommandBuffer get_frame_command_buffer(VulkanFrame& frame);
VkCommandBuffer create_command_buffer(const VkDevice& device, FamilyCapability family_pool);



};

class VulkanSyncManager
{
private:
    std::vector<VkFence> fences;
    std::vector<VkSemaphore> semaphores;

public:
    VulkanSyncManager(const VkDevice& device, const int max_frames_in_flight);
    void Free(const VkDevice& device);

    VkFence get_frame_fence(const uint32_t frame_index);
    VkFence get_frame_fence(const VulkanFrame& frame);

    VkSemaphore get_frame_semaphore(const uint32_t frame_index);
    VkSemaphore get_frame_semaphore(const VulkanFrame& frame);
};

class VulkanSwapchain
{

private:

// small functions to keep code away from constructor
void createSwapchainKHR(const VkDevice& device);

const std::vector<VkImage>& get_images(const VkDevice& device); 

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
void create_image_views(const VkDevice& device);

VkFormat get_depth_format(VkPhysicalDevice physical_device);
/*
    Creates a depth image and image view
*/
void create_depth_attachment(const VkDevice& device, VmaAllocator allocator);



public:

VkSwapchainKHR swapchain;
VkSwapchainCreateInfoKHR info{};

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
void Free(const VmaAllocator& vma, const VkDevice& device);

}; 
