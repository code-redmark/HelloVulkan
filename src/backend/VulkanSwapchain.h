#pragma once

#include "VulkanBackend.h"

#include <iostream>
#include <vector>
#include <optional>
#include <array>
#include <set>

class VulkanSwapchain
{
private:
VkSwapchainKHR swapchain;
VulkanSwapchain::VulkanSwapchain(VkDevice &device, VkPhysicalDevice &physical_device, VkSurfaceKHR &surface, std::array<std::optional<int>, capability_count()>& family_indices);

public:

friend class VulkanContext;
};