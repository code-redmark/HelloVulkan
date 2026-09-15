#include "Cleaner.h"

#include "Context.h"
#include "Swapchain.h"
#include "CommandManager.h"
#include "SyncManager.h"

VulkanBackend::Cleaner::Cleaner(Context& context)
    : context(context) {}

void VulkanBackend::Cleaner::FreeAssets()
{
    GSAM_LOG_DEBUG("Freeing meshes");
    for (const TypedResourceHandle<GpuMesh>& handle : context.meshHandles)
	{
		GpuMesh* mesh = context.meshRegistry.Get(handle);
	
		mesh->Free(context.vma);
		
		context.meshRegistry.Release(handle);
	}

	GSAM_LOG_DEBUG("Freeing images");
	for (const TypedResourceHandle<GpuImage>& handle : context.imageHandles)
	{
		auto* image = context.imageRegistry.Get(handle);
		if (image)
		{
			vmaDestroyImage(context.vma, image->image, image->allocation);
			vkDestroyImageView(context.device, image->imageView, nullptr);
		}
	}
}

void VulkanBackend::Cleaner::FreeVulkanObjects()
{
    GSAM_LOG_DEBUG("Freeing Vulkan objects");
    for (Frame frame : context.frames)
	{
		frame.Free(context.vma);
	}
}


void VulkanBackend::Cleaner::FreeManagers()
{
    GSAM_LOG_DEBUG("Freeing managers");
    context.commandManager->Free(context.device);
	context.syncManager->Free(context.device);
}

void VulkanBackend::Cleaner::FreeCore()
{
    GSAM_LOG_DEBUG("Freeing core");
	
	context.swapchain->Free(context.vma, context.device);
	
	#ifndef NDEBUG
		VmaTotalStatistics stats{};
		vmaCalculateStatistics(context.vma, &stats);
		GSAM_LOG_DEBUG("Destroying VMA (allocations left: " + std::to_string(stats.total.statistics.allocationCount) + ")");
		
		if (stats.total.statistics.allocationCount != 0) 
		{
			char* stats_string;
			vmaBuildStatsString(context.vma, &stats_string, VK_TRUE);
			GSAM_LOG_ERROR("\n" + std::string(stats_string));
		}
		vmaDestroyAllocator(context.vma);
	#endif

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

void VulkanBackend::Cleaner::FreeInstance()
{
    GSAM_LOG_DEBUG("Freeing instance");
	if (context.instance != VK_NULL_HANDLE) {
		vkDestroyInstance(context.instance, nullptr);
		context.instance = VK_NULL_HANDLE;
	}
}










