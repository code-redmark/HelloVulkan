#include "VulkanBackend.h"

#include "Context.h"
#include "Swapchain.h"

#include <set>
#include <vector>
#include <stdexcept>
#include <iostream>

VulkanBackend::Swapchain::Swapchain(Context& context)
    : swapchain(VK_NULL_HANDLE)
{
    set_queue_families(context.queue_families_indices);
    set_surface_capability_info(context.physical_device, context.surface);
    set_image_format();

    createSwapchainKHR(context.device);
    
    create_surface_image_views(context.device);

    this->depth_image_format = get_depth_format(context.physical_device);
    create_depth_attachment(context);
}



void VulkanBackend::Swapchain::set_queue_families(const std::array<std::optional<int>, GSAM::Vulkan::capability_count()>& queue_families_indices)
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

void VulkanBackend::Swapchain::set_image_format()
{
    this->image_format.format = VK_FORMAT_B8G8R8A8_SRGB;
    this->image_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
}

void VulkanBackend::Swapchain::set_surface_capability_info(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
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

void VulkanBackend::Swapchain::select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
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

void VulkanBackend::Swapchain::get_surface_images(const VkDevice& device)
{
    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(device, this->swapchain, &imgCount, nullptr);
    this->surface_images.resize(imgCount);
    vkGetSwapchainImagesKHR(device, this->swapchain, &imgCount, this->surface_images.data());

}

void VulkanBackend::Swapchain::createSwapchainKHR(const VkDevice& device)
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

    this->info.flags = 0;

    GSAM_VK_CHECK(vkCreateSwapchainKHR(device, &this->info, nullptr, &this->swapchain), "Failed to create swapchain");


    GSAM_LOG_DEBUG("Swapchain created");
}

void VulkanBackend::Swapchain::create_surface_image_views(const VkDevice& device)
{
    const auto& imgs = this->surface_images;
    this->surface_image_views.resize(imgs.size());
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

        GSAM_VK_CHECK(
            vkCreateImageView(device, &viewInfo, nullptr, &this->surface_image_views[i]), 
            "Error creating image view " + std::to_string(i)
        );

    }
}

VkFormat VulkanBackend::Swapchain::get_depth_format(VkPhysicalDevice physical_device)
{
    std::array<VkFormat, 2> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    for (VkFormat format : depthFormats) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            this->depth_image_format = format;
            break;
        }
    }

    if (this->depth_image_format == VK_FORMAT_UNDEFINED) {
        GSAM_THROW_ERROR("No suitable depth format found");
    }

    return this->depth_image_format;
}

void VulkanBackend::Swapchain::create_depth_attachment(VulkanBackend::Context& context)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = this->depth_image_format;
    // this window size shit is tormenting me
    //depthImageCI.extent = {.width = static_cast<uint32_t>(windowSize.x), .height = static_cast<uint32_t>(windowSize.y), .depth = 1 };
    imageInfo.extent = {.width = 800, .height = 600, .depth = 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo {
    .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
    .usage = VMA_MEMORY_USAGE_AUTO
    };


    GpuImage out;
    VkResult res = vmaCreateImage(
        context.vma, 
        &imageInfo, 
        &allocInfo, 
        &out.image, 
        &out.allocation, 
        nullptr
    );
    GSAM_VK_CHECK(res, "Failed to create depth image");
    GSAM_LOG_DEBUG("Depth image created");

    vmaSetAllocationName(context.vma, out.allocation, "Depth attachment image");

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = out.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = this->depth_image_format;

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    viewInfo.subresourceRange = subresourceRange;

    VkResult viewRes = vkCreateImageView(context.device, &viewInfo, nullptr, &out.imageView);
    GSAM_VK_CHECK(viewRes, "Failed to create depth image view");

    
    TypedResourceHandle<GpuImage> handle = context.imageRegistry.Allocate();
    context.imageRegistry.Set(handle, out);
    context.imageHandles.push_back(handle);

    GSAM_LOG_DEBUG("Depth image view created");
}

void VulkanBackend::Swapchain::Free(const VmaAllocator& vma, const VkDevice& device)
{
    for (VkImageView s_view : this->surface_image_views)
    {
        vkDestroyImageView(device, s_view, nullptr);
    }
    for (VkImage s_img : this->surface_images)
    {
        vkDestroyImage(device, s_img, nullptr);
    }

    vkDestroySwapchainKHR(device, this->swapchain, nullptr);
}
