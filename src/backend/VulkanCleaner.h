#pragma once

class VulkanContext;

class VulkanCleaner 
{
private:
    VulkanContext& context;

public:
    VulkanCleaner(VulkanContext& context);

    /*
        Frees meshes, textures, etc.
    */
    void FreeAssets();

    /*
        Frees Vulkan objects needed for the program to run properly,
        including command buffers, synchronization objects
    */
    void FreeVulkanObjects();

    /*
        Frees other vulkan objects that were kept inside of managers
    */
    void FreeManagers();

    /*
        Frees the swapchain, logical device and other core,
        internal structures
    */
    void FreeCore();

    /*
        Frees the VkInstance
    */
    void FreeInstance();

};