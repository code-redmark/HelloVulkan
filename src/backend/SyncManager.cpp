#include "SyncManager.h"

/*
    creates a fence and a semaphore for each frame in flight
*/
VulkanBackend::SyncManager::SyncManager(const VkDevice& device, const int max_frames_in_flight)
{
    this->frame_fences.resize(max_frames_in_flight);
    for (VkFence& fence : this->frame_fences)
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



VkFence VulkanBackend::SyncManager::get_frame_fence(const uint32_t frame_index)
{
    return this->frame_fences[frame_index];
}

VkFence VulkanBackend::SyncManager::get_frame_fence(const Frame& frame)
{
    return this->frame_fences[frame.index];
}

VkSemaphore VulkanBackend::SyncManager::get_frame_semaphore(const uint32_t frame_index)
{
    return this->semaphores[frame_index];
}
VkSemaphore VulkanBackend::SyncManager::get_frame_semaphore(const Frame& frame)
{
    return this->semaphores[frame.index];
}

TypedResourceHandle<VkFence> VulkanBackend::SyncManager::create_fence(const VkDevice& device)
{   
    auto handle = this->fence_registry.Allocate();
    
    VkFenceCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };
    
    VkFence fence;
    vkCreateFence(device, &info, nullptr, &fence);
    this->fence_registry.Set(handle, fence);
    this->fence_handles.push_back(handle);
    
    return handle;
}
VkFence* VulkanBackend::SyncManager::get_fence(TypedResourceHandle<VkFence> handle)
{
    return this->fence_registry.Get(handle);
}

void VulkanBackend::SyncManager::Free(const VkDevice& device)
{
    for (VkFence fence : this->frame_fences)
    {
        vkDestroyFence(device, fence, nullptr);
    }
    
    for (TypedResourceHandle<VkFence> handle : this->fence_handles)
    {
        VkFence* fence = this->fence_registry.Get(handle);
        if (fence == nullptr) continue;

        vkDestroyFence(device, *fence, nullptr);
    }
    for (VkSemaphore sem : this->semaphores)
    {
        vkDestroySemaphore(device, sem, nullptr);
    }

}