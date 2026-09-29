#pragma once

#include "VulkanBackend.h"

/*
    Contains all the commands in their command buffers in the
    respective command pools for the given context
*/
class VulkanBackend::CommandManager
{

friend class Context;

private:

    /*
        Contains the base command pools assigned to each queue family
        the context has picked
    */
    std::array<std::optional<VkCommandPool>, GSAM::Vulkan::capability_count()> base_pools;
    /*
        Contains the base command buffers assigned to frame in flight
    */
    std::vector<VkCommandBuffer> base_buffers;

    TypedResourceRegistry<VkCommandPool> cmdPoolRegistry;
    std::vector<TypedResourceHandle<VkCommandPool>> cmdPoolHandles;
    
    public:
    
    CommandManager(std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices, const VkDevice& device, const int max_frames_in_flight);
    void Free(const VkDevice& device);
    
    VkCommandBuffer get_frame_command_buffer(Frame& frame);
    VkCommandPool get_family_pool(GSAM::Vulkan::QueueFamilyCapability family_capability);

    TypedResourceHandle<VkCommandPool> create_command_pool(const VkDevice& device, const uint32_t queue_family_index);
    
    VkCommandBuffer create_command_buffer(const VkDevice& device, GSAM::Vulkan::QueueFamilyCapability family_capability);
    std::vector<VkCommandBuffer> create_command_buffers(const VkDevice& device, GSAM::Vulkan::QueueFamilyCapability family_capability, uint32_t count);
    
    

};