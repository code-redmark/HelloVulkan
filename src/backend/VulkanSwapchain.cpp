#include "VulkanContext.h"



VulkanSwapchain::VulkanSwapchain(VulkanContext& context)
    : swapchain(VK_NULL_HANDLE)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physical_device, context.surface, &surface_capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context.physical_device, context.surface, &format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context.physical_device, context.surface, &format_count, formats.data());

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.physical_device, context.surface, &present_mode_count, nullptr);

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.physical_device, context.surface, &present_mode_count, present_modes.data());

    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.clipped = VK_TRUE;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.imageArrayLayers = 1;
    info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    info.imageExtent = surface_capabilities.currentExtent;
    info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;

    std::cout << "starting std::set stuff\n";

    // find the unique indices in the application's queue families
    std::set<int> indices;
    std::cout << "(size = " << context.queue_families_indices.size() << ")";
    std::cout << "Family index indices: \n";
    for (int i = 0; i < context.queue_families_indices.size(); i++)
    {
        if (context.queue_families_indices[i].has_value())
        {
            indices.insert(context.queue_families_indices[i].value());
            std::cout << "queue_families_indices[" << i << "] = " << context.queue_families_indices[i].value() << std::endl;
        }
    }

    std::cout << "\nfinished std::set stuff\n";


    info.queueFamilyIndexCount = indices.size() > 1 ? indices.size() : 0;
    std::vector<uint32_t> indices_list(indices.begin(), indices.end());

    std::cout << "List size: " << indices_list.size() << "\nset size: " <<  indices.size() << "\n";

    if (indices.size() > 1)
    {
        std::cout << "[VulkanSwapchain::VulkanSwapchain] INFO: Sharing mode is concurrent\n";
        
        
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.pQueueFamilyIndices = indices_list.data();
    } else
    {
        std::cout << "Sharing mode is exclusive\n";
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    

    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.oldSwapchain = VK_NULL_HANDLE;
    info.preTransform = surface_capabilities.currentTransform;
    
    uint32_t imageCount = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && imageCount > surface_capabilities.maxImageCount)
    {
        imageCount = surface_capabilities.maxImageCount;
    }
    info.minImageCount = imageCount;
    info.pNext = nullptr;

    /*
        This is totally temporary and i plan making something like profiles for the vulkan
        backend in GSAM to give the user some of the freedom vulkan is about
    */
    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    
    std::cout << "VK_FORMAT_B8G8R8A8_SRGB = " << VK_FORMAT_B8G8R8A8_SRGB << std::endl;
    std::cout << "VK_FORMAT_R8G8B8A8_SRGB = " << VK_FORMAT_R8G8B8A8_SRGB << std::endl;
    std::cout << "VK_FORMAT_R8G8B8A8_UNORM = " << VK_FORMAT_R8G8B8A8_UNORM << std::endl;
    std::cout << "VK_PRESENT_MODE_FIFO_KHR = " << VK_PRESENT_MODE_FIFO_KHR << std::endl;
    std::cout << "VK_PRESENT_MODE_MAILBOX_KHR = " << VK_PRESENT_MODE_MAILBOX_KHR << std::endl;

    std::cout << "format: " << info.imageFormat << "\n";
    std::cout << "extent: " << info.imageExtent.width << "x" << info.imageExtent.height << "\n";
    std::cout << "minImageCount: " << info.minImageCount << "\n";
    std::cout << "presentMode: " << info.presentMode << "\n";
    std::cout << "compositeAlpha: " << info.compositeAlpha << "\n";
    std::cout << "preTransform: " << info.preTransform << "\n";
    std::cout << "sharingMode: " << info.imageSharingMode << "\n";
    std::cout << "queueFamilyIndexCount: " << info.queueFamilyIndexCount << "\n";

    vkDeviceWaitIdle(context.device);
    VkResult result = vkCreateSwapchainKHR(context.device, &info, nullptr, &this->swapchain);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VulkanSwapchain::VulkanSwapchain] ERROR: swapchain creation failed\n";
    }

} 