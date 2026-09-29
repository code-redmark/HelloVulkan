#pragma once

#include "VulkanBackend.h"

class VulkanBackend::SyncManager
{
private:
    /*
        Fences reserved for each frame in flight
    */
    std::vector<VkFence> frame_fences;

    /*
        Semaphores reserved for each frame in flight
    */
    std::vector<VkSemaphore> semaphores;
    
    TypedResourceRegistry<VkFence> fence_registry;
    std::vector<TypedResourceHandle<VkFence>> fence_handles;
public:
    SyncManager(const VkDevice& device, const int max_frames_in_flight);
    void Free(const VkDevice& device);

    VkFence get_frame_fence(const uint32_t frame_index);
    VkFence get_frame_fence(const Frame& frame);

    VkSemaphore get_frame_semaphore(const uint32_t frame_index);
    VkSemaphore get_frame_semaphore(const Frame& frame);

    TypedResourceHandle<VkFence> create_fence(const VkDevice& device);
    VkFence* get_fence(TypedResourceHandle<VkFence> handle);

};