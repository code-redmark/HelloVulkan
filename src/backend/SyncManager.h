#pragma once

#include "VulkanBackend.h"

class VulkanBackend::SyncManager
{
private:
    std::vector<VkFence> fences;
    std::vector<VkSemaphore> semaphores;

public:
    SyncManager(const VkDevice& device, const int max_frames_in_flight);
    void Free(const VkDevice& device);

    VkFence get_frame_fence(const uint32_t frame_index);
    VkFence get_frame_fence(const Frame& frame);

    VkSemaphore get_frame_semaphore(const uint32_t frame_index);
    VkSemaphore get_frame_semaphore(const Frame& frame);
};