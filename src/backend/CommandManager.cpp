#include "CommandManager.h"

VulkanBackend::CommandManager::CommandManager(std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices, const VkDevice &device, const int max_frames_in_flight)
{
    this->base_buffers.resize(max_frames_in_flight);
    try
    {
        for (int i = 0; i < queue_families_indices.size(); i++)
        {
            if (!queue_families_indices[i].has_value())
                continue;

            VkCommandPoolCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = static_cast<uint32_t>(queue_families_indices[i].value()),
            };

            VkCommandPool pool;
            VkResult res = vkCreateCommandPool(device, &info, nullptr, &pool);
            GSAM_VK_CHECK(res, "Couldn't create command pool");
            this->base_pools[i] = pool;
        }
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << "\n";
    }
}

TypedResourceHandle<VkCommandPool> VulkanBackend::CommandManager::create_command_pool(const VkDevice &device, const uint32_t queue_family_index)
{
    VkCommandPoolCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = static_cast<uint32_t>(queue_family_index)
    };

    VkCommandPool pool;
    VkResult res = vkCreateCommandPool(device, &info, nullptr, &pool);
    GSAM_VK_CHECK(res, "Couldn't create command pool");

    auto handle = this->cmdPoolRegistry.Allocate();
    this->cmdPoolRegistry.Set(handle, pool);

    return handle;
}

VkCommandBuffer VulkanBackend::CommandManager::get_frame_command_buffer(Frame &frame)
{
    if (frame.index >= this->base_buffers.size())
    {
        GSAM_THROW_ERROR("frame index " + std::to_string(frame.index) + " is out of bounds for command buffer array");
    }

    return this->base_buffers[frame.index];
}

VkCommandPool VulkanBackend::CommandManager::get_family_pool(GSAM::Vulkan::QueueFamilyCapability family_capability)
{
    if (!this->base_pools[enumtoi(family_capability)].has_value())
    {
        GSAM_THROW_ERROR("command pool for family " + std::to_string(enumtoi(family_capability)) + " doesn't exist");
    }

    return this->base_pools[enumtoi(family_capability)].value();
}

VkCommandBuffer VulkanBackend::CommandManager::create_command_buffer(const VkDevice &device, GSAM::Vulkan::QueueFamilyCapability family_capability)
{
    VkCommandPool pool = this->get_family_pool(family_capability);

    VkCommandBufferAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    VkCommandBuffer data;
    VkResult res = vkAllocateCommandBuffers(device, &info, &data);
    GSAM_VK_CHECK(res, "Couldn't allocate command buffer for family " + std::to_string(enumtoi(family_capability)));

    return data;
}

std::vector<VkCommandBuffer> VulkanBackend::CommandManager::create_command_buffers(const VkDevice &device, GSAM::Vulkan::QueueFamilyCapability family_pool, uint32_t count)
{
    if (!this->base_pools[enumtoi(family_pool)].has_value())
    {
        GSAM_THROW_ERROR("command pool for family " + std::to_string(enumtoi(family_pool)) + " doesn't exist");
    }

    VkCommandBufferAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = this->base_pools[enumtoi(family_pool)].value(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count};

    std::vector<VkCommandBuffer> buffers(count);

    VkResult res = vkAllocateCommandBuffers(device, &info, buffers.data());
    GSAM_VK_CHECK(res, "Couldn't allocate command buffer for family " + std::to_string(enumtoi(family_pool)));

    return buffers;
}

void VulkanBackend::CommandManager::Free(const VkDevice &device)
{
    for (std::optional<VkCommandPool> pool : this->base_pools)
    {
        if (!pool.has_value())
            continue;

        vkDestroyCommandPool(device, pool.value(), nullptr);
    }
    for (TypedResourceHandle<VkCommandPool> handle : this->cmdPoolHandles)
    {
        VkCommandPool *pool = this->cmdPoolRegistry.Get(handle);
        vkDestroyCommandPool(device, *pool, nullptr);
    }
}