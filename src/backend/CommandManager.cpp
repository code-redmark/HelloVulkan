#include "VulkanContext.h"

CommandManager::CommandManager(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device)
{
    try
    {
        create_command_pools(queue_families_indices, device);


    } catch (const std::runtime_error& err)
    {
        std::cerr << "[CommandManager::CommandManager] ERROR: " << err.what() << "\n";
    }
}

void CommandManager::create_command_pools(std::array<std::optional<int>, capability_count()> queue_families_indices, const VkDevice& device)
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
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("[CommandManager::CommandManager] ERROR: couldn't create command pool for family " + i);
        }
        
        this->pools[i] = pool;
    }

    std::cout << "[CommandManager::create_command_pools] OK: created command pools\n";
}
