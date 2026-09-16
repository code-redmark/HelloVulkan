#pragma once

#include "VulkanBackend.h"

#include "Swapchain.h"
#include "CommandManager.h"
#include "SyncManager.h"
#include "Cleaner.h"

class VulkanBackend::Context
{

friend class VulkanBackend::Swapchain;
friend class VulkanBackend::Cleaner;

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

    std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices;
    std::unique_ptr<Swapchain> swapchain = nullptr;
    
    #ifndef NDEBUG
    bool check_validation_layers_support();
    void create_debug_messenger();
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
    void create_device(GSAM::Vulkan::ApplicationRequirements& requirements);
    
    /*
        Creates a VMA Allocator
    */
    void setup_vma();

    /*
        Creates [MAX_FRAMES_IN_FLIGHT] VulkanFrames, assumes the instance's commandManager and syncManager
        have already been initialized and will be produce errors if called before
    */
    void create_frames();

    TypedResourceRegistry<GpuBuffer> bufferRegistry;
    std::vector<TypedResourceHandle<GpuBuffer>> bufferHandles;

    TypedResourceHandle<GpuBuffer> CreateBuffer(
        const VkBufferCreateInfo& buffer_create_info,
        const VmaAllocationCreateInfo& allocation_create_info
    );

    TypedResourceRegistry<GpuImage> imageRegistry;
    std::vector<TypedResourceHandle<GpuImage>> imageHandles;
    
    TypedResourceHandle<GpuImage> CreateImage(
        const VkImageCreateInfo image_create_info, 
        const VmaAllocationCreateInfo& allocation_create_info,
        VkImageViewCreateInfo& image_view_create_info
    );

    TypedResourceRegistry<GpuMesh> meshRegistry;
    std::vector<TypedResourceHandle<GpuMesh>> meshHandles;

    std::unique_ptr<CommandManager> commandManager;
    std::unique_ptr<SyncManager> syncManager;
    
    std::vector<Frame> frames;
    std::optional<Frame> CreateFrame();

    std::unique_ptr<Cleaner> cleaner;

public:
    Context(void* window_handle, GSAM::Vulkan::ApplicationRequirements &requirements);
    void shutdown();


    /*
        Engine-ish stuff thats not making it into the RHI but that i need for this renderer  
    */

    GSAM::GSMesh CreateMesh(const std::filesystem::path& path);
    GSAM::GSTexture CreateTexture(const ImageData& data);
};







