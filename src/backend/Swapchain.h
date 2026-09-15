#pragma once

#include "VulkanBackend.h"


class VulkanBackend::Swapchain
{

private:

// small functions to keep code away from constructor
void createSwapchainKHR(const VkDevice& device);

void get_surface_images(const VkDevice& device); 

void set_queue_families(const std::array<std::optional<int>, GSAM::Vulkan::capability_count()>& queue_families_indices);

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
void create_surface_image_views(const VkDevice& device);

VkFormat get_depth_format(VkPhysicalDevice physical_device);
/*
    Creates a depth image and image view
*/
void create_depth_attachment(VulkanBackend::Context& context);



public:

VkSwapchainKHR swapchain;
VkSwapchainCreateInfoKHR info{};

VkSharingMode sharing_mode;
VkSurfaceFormatKHR image_format;
VkColorSpaceKHR image_color_space;

std::vector<VkImage> surface_images;
std::vector<VkImageView> surface_image_views; 


VkFormat depth_image_format;
TypedResourceHandle<GpuImage> depth_image;

Swapchain(Context& context);
void Free(const VmaAllocator& vma, const VkDevice& device);

}; 