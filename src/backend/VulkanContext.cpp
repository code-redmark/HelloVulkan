#include "VulkanContext.h"
#include "VulkanBackend.h"
#include <iostream>

#include <optional>



VulkanContext::VulkanContext(void* window_handle, ApplicationRequirements &requirements)
	: instance(VK_NULL_HANDLE), physical_device(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), device(VK_NULL_HANDLE), queue_families_indices({std::nullopt}), swapchain(nullptr)
{
	try  
	{
		create_instance();

		#ifndef NDEBUG
			if (!create_debug_messenger())
			{
				GSAM_THROW_ERROR("create debug messenger");
			} else GSAM_LOG_DEBUG("Created debug messenger");
		#endif

		pick_device();

		create_surface(window_handle);

		create_device(requirements);

		setup_vma();

		this->swapchain = std::make_unique<VulkanSwapchain>(*this);

	}
	catch (const std::runtime_error& err)
	{
		std::cerr << err.what() << std::endl;
		exit(-1);
	}


}

void VulkanContext::shutdown()
{

	this->swapchain->~VulkanSwapchain();

	if (this->device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(this->device, nullptr);
	}
	if (this->surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
	}

	PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger =
    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );

	if (destroyDebugMessenger)
	{
		destroyDebugMessenger(instance, this->debugMessenger, nullptr);
	}

	if (this->instance != VK_NULL_HANDLE) {
		vkDestroyInstance(this->instance, nullptr);
		this->instance = VK_NULL_HANDLE;
	}
}

void VulkanContext::create_instance()
{
	std::vector<const char*> extensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	#if defined(_WIN32)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	#elif defined(__ANDROID__)
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);

	#elif defined(__linux__)

		#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
			extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_XCB_KHR)
			extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_XLIB_KHR)
			extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
		#endif

	#elif defined(__APPLE__)
		extensions.push_back(VK_KHR_METAL_SURFACE_EXTENSION_NAME);
	#endif


	std::vector<const char*> layers;
	#ifndef NDEBUG
		if (this->check_validation_layers_support()) layers.push_back("VK_LAYER_KHRONOS_validation");
			else GSAM_LOG_DEBUG("Validation layers not supported");
		GSAM_LOG_DEBUG("pushed VK_LAYER_KHRONOS_validation");
	#endif

	VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "67";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "triangel";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();


	VkResult instanceResult = vkCreateInstance(&createInfo, nullptr, &this->instance);
	GSAM_VK_CHECK(instanceResult, "Failed to create VkInstance");
}

void VulkanContext::pick_device()
{
	uint32_t count;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);

	std::vector<VkPhysicalDevice> pDevices(count);
	vkEnumeratePhysicalDevices(instance, &count, pDevices.data());

	std::optional<std::string> selected_name = std::nullopt;

	for (int i = 0; i < count; i++)
	{
		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(pDevices[i], &prop);

		if (this->physical_device == VK_NULL_HANDLE && prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			this->physical_device = pDevices[i];
			selected_name = prop.deviceName;
			break;
		}
	}

	if (selected_name.has_value()) {
		GSAM_LOG_INFO("Selected discrete GPU: " + selected_name.value());
		return;
	}

	if (this->physical_device == VK_NULL_HANDLE) 
	{
		if (!pDevices.empty())
		{
			GSAM_LOG_INFO("No discrete GPU found. Falling back to first device found");
			this->physical_device = pDevices[0];
		}
		else GSAM_THROW_ERROR("No physical device found");
	}

}

/*
	Creates a surface, a connection between Vulkan and our window handle,
	the window handle can come from any window library, in fact it is a
	void pointer
*/
void VulkanContext::create_surface(void* win_handle)
{
	VkWin32SurfaceCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.pNext = nullptr;
	info.flags = 0;
	info.hinstance = GetModuleHandle(nullptr);
	info.hwnd = (HWND)win_handle;

	VkResult creationResult = vkCreateWin32SurfaceKHR(this->instance, &info, nullptr, &this->surface);
	GSAM_VK_CHECK(creationResult, "Failed to create Win32 surface");
}

void VulkanContext::create_device(ApplicationRequirements& requirements)
{
	uint32_t fam_count;
	vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &fam_count, nullptr);

	// array of single family property objects
	std::vector<VkQueueFamilyProperties> fams_props(fam_count);
	vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &fam_count, fams_props.data());

	std::vector<VkDeviceQueueCreateInfo> qInfos;

	std::vector<std::vector<float>> priorities;
	for (int i = 0; i < fam_count; i++)
	{
		bool used = false;
		int q_count = -1;
		if (requirements.requires(FamilyCapability::Graphics))
		{
			if (!this->queue_families_indices[enum_index(FamilyCapability::Graphics)].has_value() && 
			fams_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				this->queue_families_indices[enum_index(FamilyCapability::Graphics)] = i;

				used = true;
				int count = requirements.queue_requirement(FamilyCapability::Presentation);
				if (count > q_count) q_count = count; 

			}
		}
		
		if (requirements.requires(FamilyCapability::Presentation))
		{
			if (!this->queue_families_indices[enum_index(FamilyCapability::Presentation)].has_value())
			{
				
				VkBool32 supported = VK_FALSE;
				VkResult requestResult = vkGetPhysicalDeviceSurfaceSupportKHR(
					this->physical_device,
					i,
					this->surface,
					&supported
				);
			
				if (supported == VK_TRUE && requestResult == VK_SUCCESS)
				{
					this->queue_families_indices[enum_index(FamilyCapability::Presentation)] = i;	
				} 

				used = true;
				int count = requirements.queue_requirement(FamilyCapability::Presentation);
				if (count > q_count) q_count = count;
			}
		}
		
		if (used)
		{
			priorities.push_back(std::vector<float>(q_count, 1.f));

			qInfos.emplace_back();
			auto& back = qInfos.back();
			back.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			back.queueCount = q_count;
			back.queueFamilyIndex = i;
			back.pQueuePriorities = priorities.back().data();
		}

	}

	for (int i = 0; i < enum_index(FamilyCapability::Count); i++)
	{
		if (requirements.requires(index_enum<FamilyCapability>(i)) && !this->queue_families_indices[i].has_value()) 
		{
			GSAM_THROW_ERROR("Available queue families couldn't satisfy application requirements");
		}

	}

	VkPhysicalDeviceVulkan12Features Vk12Features{};
	Vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	Vk12Features.descriptorIndexing = true;
	Vk12Features.shaderSampledImageArrayNonUniformIndexing = true;
	Vk12Features.descriptorBindingVariableDescriptorCount = true;
	Vk12Features.runtimeDescriptorArray = true;
	Vk12Features.bufferDeviceAddress = true;

	VkPhysicalDeviceVulkan13Features Vk13Features{};
	Vk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	Vk13Features.pNext = &Vk12Features;
	Vk13Features.synchronization2 = true;
	Vk13Features.dynamicRendering = true;

	VkPhysicalDeviceFeatures Vk10Features{};
	Vk10Features.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	info.pEnabledFeatures = &Vk10Features;
	info.pNext = &Vk13Features;

	info.queueCreateInfoCount = qInfos.size();
	info.pQueueCreateInfos = qInfos.data();

	const char* names[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	info.ppEnabledExtensionNames = names;
	info.enabledExtensionCount = static_cast<uint32_t>(std::size(names));

	info.pEnabledFeatures = nullptr;
	info.flags = 0;

	VkResult deviceResult = vkCreateDevice(this->physical_device, &info, nullptr, &this->device);
	GSAM_VK_CHECK(deviceResult, "Failed to create logical device");
	
}

void VulkanContext::setup_vma()
{
	VmaVulkanFunctions vkFunctions{};
	vkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;;
	vkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vkFunctions.vkCreateImage = vkCreateImage;
	
	VmaAllocatorCreateInfo info{};
	info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	info.physicalDevice = this->physical_device;
	info.device = this->device;
	info.pVulkanFunctions = &vkFunctions;
	info.instance = instance;

	VkResult res = vmaCreateAllocator(&info, &this->vma);
	GSAM_VK_CHECK(res, "Failed to create VMA Allocator");
}


