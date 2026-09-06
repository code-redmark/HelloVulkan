#include "VulkanContext.h"

#include <set>
#include <vector>
#include <stdexcept>
#include <iostream>

VulkanSwapchain::VulkanSwapchain(VulkanContext& context)
    : swapchain(VK_NULL_HANDLE), device(context.device)
{
    try
    {
        set_queue_families(context.queue_families_indices);
        set_surface_capability_info(context.physical_device, context.surface);
        set_image_format();

        createSwapchainKHR();
        
        create_image_views();
    }

    catch(const std::runtime_error& err)
    {
        std::cerr << err.what() << '\n';
    }
}

VulkanSwapchain::~VulkanSwapchain()
{
    for (VkImageView view : this->image_views)
    {
        vkDestroyImageView(this->device, view, nullptr);
    }

    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
}

void VulkanSwapchain::set_queue_families(const std::array<std::optional<int>, capability_count()>& queue_families_indices)
{
    if (queue_families_indices.size() < 1) GSAM_THROW_ERROR("Invalid queue_families_indices passed (size = " + std::to_string(queue_families_indices.size()) + ")");
        
    std::set<uint32_t> uniqueIndices;
    for (const auto& optIndex : queue_families_indices) {
        if (optIndex.has_value()) {
            uniqueIndices.insert(optIndex.value());
        }
    }
    std::vector<uint32_t> indicesList(uniqueIndices.begin(), uniqueIndices.end());

    this->info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    const uint32_t* pQueueFamilyIndices = nullptr;

    if (uniqueIndices.size() > 1) {
        GSAM_LOG_DEBUG("Sharing mode is concurrent\n");
        this->info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        this->info.queueFamilyIndexCount = static_cast<uint32_t>(indicesList.size());
        pQueueFamilyIndices = indicesList.data();
    } else {
        GSAM_LOG_DEBUG("Sharing mode is exclusive");
    }

    this->info.pQueueFamilyIndices = pQueueFamilyIndices;
}

void VulkanSwapchain::set_image_format()
{
    this->image_format.format = VK_FORMAT_B8G8R8A8_SRGB;
    this->image_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

void VulkanSwapchain::set_surface_capability_info(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    this->info.surface = surface;

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surfaceCaps);
    GSAM_VK_CHECK(res, "Couldn't get surface capabilities")
    
    this->info.imageExtent = surfaceCaps.currentExtent;
    if (this->info.imageExtent.width == 0xFFFFFFFF) {
        // TODO: get window size
        // auto winSize = context.getWindowSize();
        // extent.width = static_cast<uint32_t>(winSize.x);
        // extent.height = static_cast<uint32_t>(winSize.y);
        

        this->info.imageExtent.width = 800; // temporary
        this->info.imageExtent.height = 600;
    }

    this->info.minImageCount = surfaceCaps.minImageCount;
    if (surfaceCaps.maxImageCount > 0 && surfaceCaps.minImageCount > surfaceCaps.maxImageCount) {
        this->info.minImageCount = surfaceCaps.maxImageCount;
    }

    this->info.preTransform = surfaceCaps.currentTransform;

}

void VulkanSwapchain::select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    uint32_t presentModeCount = 0;
    VkResult res = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, nullptr);
    
    GSAM_VK_CHECK(res, "Couldn't get present modes");

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, presentModes.data());

    /*
        TODO: make some sort of enum or mode that the user can edit to make
        the present mode choice based on the application's requirements (VsyncMode for example)
    */
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (VkPresentModeKHR mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }
}

const std::vector<VkImage>& VulkanSwapchain::get_images()
{
    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &imgCount, nullptr);
    this->images.resize(imgCount);
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &imgCount, this->images.data());

    return this->images;
}

void VulkanSwapchain::createSwapchainKHR()
{
    this->info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    this->info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    this->info.imageFormat = this->image_format.format;
    this->info.imageColorSpace = this->image_format.colorSpace;
    
    this->info.imageArrayLayers = 1;
    this->info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    this->info.clipped = VK_TRUE;
    this->info.oldSwapchain = VK_NULL_HANDLE;
    this->info.pNext = nullptr;

    GSAM_VK_CHECK(vkCreateSwapchainKHR(this->device, &this->info, nullptr, &this->swapchain), "Failed to create swapchain")


    GSAM_LOG_DEBUG("Swapchain created");
}

void VulkanSwapchain::create_image_views()
{
    const std::vector<VkImage>& imgs = this->get_images();   

    this->image_views.resize(imgs.size());
    for (uint32_t i = 0; i < imgs.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = imgs[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = this->image_format.format;
        
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        GSAM_VK_CHECK (
            vkCreateImageView(this->device, &viewInfo, nullptr, &this->image_views[i]), 
            "Error creating image view " + std::to_string(i)
        );

    }
}
