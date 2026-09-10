#include "VulkanContext.h"

VulkanCleaner::VulkanCleaner(VulkanContext& context)
    : context(context) {}

void VulkanCleaner::FreeAssets()
{
    GSAM_LOG_DEBUG("Freeing assets");
    for (const ResourceHandle& handle : context.meshHandles)
	{
		VulkanGpuMesh* mesh = context.meshRegistry.Get<VulkanGpuMesh>(handle);
	
		mesh->Free(context.vma);
		
		context.meshRegistry.Release(handle);
	}
}

void VulkanCleaner::FreeVulkanObjects()
{
    GSAM_LOG_DEBUG("Freeing Vulkan objects");
    for (VulkanFrame frame : context.frames)
	{
		frame.Free(context.vma);
	}
}


void VulkanCleaner::FreeManagers()
{
    GSAM_LOG_DEBUG("Freeing managers");
    context.commandManager->Free(context.device);
	context.syncManager->Free(context.device);
}

void VulkanCleaner::FreeCore()
{
    GSAM_LOG_DEBUG("Freeing core");
	
	context.swapchain->Free(context.vma, context.device);
	
	vmaDestroyAllocator(context.vma);
	
	if (context.device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(context.device, nullptr);
	}
	if (context.surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
	}

	PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger =
    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT")
    );
	if (destroyDebugMessenger)
	{
		destroyDebugMessenger(context.instance, context.debugMessenger, nullptr);
	}
}

void VulkanCleaner::FreeInstance()
{
    GSAM_LOG_DEBUG("Freeing instance");
	if (context.instance != VK_NULL_HANDLE) {
		vkDestroyInstance(context.instance, nullptr);
		context.instance = VK_NULL_HANDLE;
	}
}










