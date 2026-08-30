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
    info.surface = context.surface;


    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && 
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosenFormat = f;
            break;
        }
    }
    info.imageFormat = chosenFormat.format;

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
        info.pQueueFamilyIndices = nullptr;
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

    VkBool32 supported = VK_FALSE;
    VkResult testResult = vkGetPhysicalDeviceSurfaceSupportKHR(
        context.physical_device, 
        indices_list.empty() ? 0 : indices_list[0], 
        context.surface, 
        &supported);

    std::cout << "Surface support test result: " << testResult 
        << " supported: " << supported << "\n";

    std::cout << "sizeof(VkSwapchainCreateInfoKHR) = " << sizeof(VkSwapchainCreateInfoKHR) << "\n";
    std::cout << "info.sType = " << info.sType 
            << " (expected " << VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR << ")\n";
    std::cout << "info.pNext = " << info.pNext << "\n";
    std::cout << "&info = " << &info << "\n";

    VkResult result = vkCreateSwapchainKHR(context.device, &info, nullptr, &this->swapchain);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VulkanSwapchain::VulkanSwapchain] ERROR: swapchain creation failed\n";
    }

    uint32_t imgCount;
    VkResult imgResult = vkGetSwapchainImagesKHR(context.device, this->swapchain, &imgCount, nullptr);


    this->images.reserve(imgCount);
    imgResult = vkGetSwapchainImagesKHR(context.device, this->swapchain, &imgCount, this->images.data());

    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseArrayLayer = 0;
    range.baseMipLevel = 0;
    range.layerCount = 1;
    range.levelCount = 1;
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = this->image_format;
    viewInfo.subresourceRange = range;

    for (int i = 0; i < this->images.size(); i++)
    {
        this->image_views.emplace_back();
        VkResult imgViewCreationResult = vkCreateImageView(context.device, &viewInfo, nullptr, &this->image_views.back());
    
        if (imgViewCreationResult != VK_SUCCESS)
        {
            std::cerr << "[VulkanSwapchain::VulkanSwapchain] ERROR: error creating image view " << i << "\n";
        }
    }

    std::cout << "[VulkanSwapchain::VulkanSwapchain] OK: created image views\n";

} 