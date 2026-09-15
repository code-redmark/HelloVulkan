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
        A command pool is created for every queue family our 
        context picked up
    */
    std::array<std::optional<VkCommandPool>, GSAM::Vulkan::capability_count()> pools;
    
    std::vector<VkCommandBuffer> commandBuffers;

    /*
        Creates a command pool for each of the context's queue
        families
    */
    void create_command_pools(std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices, const VkDevice& device);
    
public:

CommandManager(std::array<std::optional<int>, GSAM::Vulkan::capability_count()> queue_families_indices, const VkDevice& device, const int max_frames_in_flight);
void Free(const VkDevice& device);

VkCommandBuffer get_frame_command_buffer(Frame& frame);
VkCommandBuffer create_command_buffer(const VkDevice& device, GSAM::Vulkan::QueueFamilyCapability family_pool);



};