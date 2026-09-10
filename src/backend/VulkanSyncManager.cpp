#include "VulkanContext.h"

/*
    creates a fence and a semaphore for each frame in flight
*/
VulkanSyncManager::VulkanSyncManager(const VkDevice& device, const int max_frames_in_flight)
{
    this->fences.resize(max_frames_in_flight);
    for (VkFence& fence : this->fences)
    {
        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        GSAM_VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &fence), "Couldn't create fence");
    }

    this->semaphores.resize(max_frames_in_flight);
    for (VkSemaphore& sem : this->semaphores)
    {
        VkSemaphoreCreateInfo semInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        GSAM_VK_CHECK(vkCreateSemaphore(device, &semInfo, nullptr, &sem), "Couldn't create semaphore");
    }

    GSAM_LOG_DEBUG("Initialized Synchonization Objects");
}

void VulkanSyncManager::Free(const VkDevice& device)
{
    for (VkFence fence : this->fences)
    {
        vkDestroyFence(device, fence, nullptr);
    }
    for (VkSemaphore sem : this->semaphores)
    {
        vkDestroySemaphore(device, sem, nullptr);
    }
}

VkFence VulkanSyncManager::get_frame_fence(const uint32_t frame_index)
{
    return this->fences[frame_index];
}

VkFence VulkanSyncManager::get_frame_fence(const VulkanFrame& frame)
{
    return this->fences[frame.index];
}

VkSemaphore VulkanSyncManager::get_frame_semaphore(const uint32_t frame_index)
{
    return this->semaphores[frame_index];
}
VkSemaphore VulkanSyncManager::get_frame_semaphore(const VulkanFrame& frame)
{
    return this->semaphores[frame.index];
}