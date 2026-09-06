#include "VulkanContext.h"

VulkanCommandManager::VulkanCommandManager(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device)
{
    try
    {
        create_command_pools(queue_families_indices, device);
    } catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << "\n";
    }
}

void VulkanCommandManager::create_command_pools(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device)
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
