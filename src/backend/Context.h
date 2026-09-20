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
    #ifdef __APPLE__
    void create_surface_apple(void* win_handle);
    #endif
    #ifdef _WIN32
    void create_surface_win32(void* win_handle);
    #endif
    #ifdef __linux__
        #ifdef VK_USE_PLATFORM_XCB_KHR
        void create_surface_xcb(void* win_handle);
        #endif
        #ifdef VK_USE_PLATFORM_XLIB_KHR
        void create_surface_xlib(Display* display, Window window);
        #endif
        #ifdef VK_USE_PLATFORM_WAYLAND_KHR
        void create_surface_wayland(wl_display* display, wl_surface* surface);
        #endif
    #endif

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
    #if defined(_WIN32)
        Context(HWND win_handle, GSAM::Vulkan::ApplicationRequirements& requirements);
    #elif defined(__APPLE__)
            Context(CAMetalLayer* win_handle, GSAM::Vulkan::ApplicationRequirements& requirements);
    #elif defined(__linux__)
        #ifdef VK_USE_PLATFORM_WAYLAND_KHR
            #include <wayland-client.h>
            Context(wl_display* display, wl_surface* surface, GSAM::Vulkan::ApplicationRequirements& requirements);
        #endif

        #ifdef VK_USE_PLATFORM_XCB_KHR
            Context(xcb_connection_t* connection, xcb_window_t window, GSAM::Vulkan::ApplicationRequirements& requirements);
        #endif

        #ifdef VK_USE_PLATFORM_XLIB_KHR
            Context(Display* display, Window window, GSAM::Vulkan::ApplicationRequirements& requirements);
        #endif
    #endif
    void shutdown();


    /*
        Engine-ish stuff thats not making it into the RHI but that i need for this renderer  
    */

    GSAM::GSMesh CreateMesh(const std::filesystem::path& path);
    GSAM::GSTexture CreateTexture(const ImageData& data);
};







