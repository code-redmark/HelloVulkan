#include "CommandManager.h"

VulkanBackend::CommandManager::CommandManager(std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices, const VkDevice& device, const int max_frames_in_flight)
{
    this->commandBuffers.resize(max_frames_in_flight);

    try
    {
        create_command_pools(queue_families_indices, device);
    } catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << "\n";
    }
}



void VulkanBackend::CommandManager::create_command_pools(std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices, const VkDevice& device)
{
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = 0;
    info.pNext = nullptr;    

    for (int i = 0; i < this->pools.size(); i++)
    {
        if (!queue_families_indices[i].has_value()) continue;
    
        VkCommandPool pool;
        
        info.queueFamilyIndex = static_cast<uint32_t>(queue_families_indices[i].value()); 

        VkResult res = vkCreateCommandPool(device, &info, nullptr, &pool);
        GSAM_VK_CHECK(res, "create command pool for family " + std::to_string(i));

        this->pools[i] = pool;
    }

    GSAM_LOG_DEBUG("created command pools");
}

VkCommandBuffer VulkanBackend::CommandManager::get_frame_command_buffer(Frame& frame)
{
    if (frame.index >= this->commandBuffers.size())
    {
        GSAM_THROW_ERROR("frame index " + std::to_string(frame.index) + " is out of bounds for command buffer array");
    }

    return this->commandBuffers[frame.index];
}

VkCommandBuffer VulkanBackend::CommandManager::create_command_buffer(const VkDevice& device, GSAM::Vulkan::QueueFamilyCapability family_pool)
{
    if (!this->pools[enum_index(family_pool)].has_value())
    {
        GSAM_THROW_ERROR("command pool for family " + std::to_string(enum_index(family_pool)) + " doesn't exist");
    }

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = this->pools[enum_index(family_pool)].value();
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

    VkCommandBuffer buffer;

    VkResult res = vkAllocateCommandBuffers(device, &info, &buffer);
    GSAM_VK_CHECK(res, "Couldn't allocate command buffer for family " + std::to_string(enum_index(family_pool)));

    return buffer;
}

void VulkanBackend::CommandManager::Free(const VkDevice& device)
{
    for (std::optional<VkCommandPool> pool : this->pools)
    {
        if (!pool.has_value()) continue;

        vkDestroyCommandPool(device, pool.value(), nullptr);
    }
}