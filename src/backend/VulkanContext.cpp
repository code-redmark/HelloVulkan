#include "VulkanContext.h"
#include "VulkanBackend.h"
#include <iostream>

#include <optional>



VulkanContext::VulkanContext(void* window_handle, ApplicationRequirements &requirements)
	: instance(VK_NULL_HANDLE), physical_device(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), device(VK_NULL_HANDLE), queue_families_indices({std::nullopt}), swapchain(nullptr)
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

	VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "67";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "triangel";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	#ifndef NDEBUG
		if (this->check_validation_layers_support()) layers.push_back("VK_LAYER_KHRONOS_validation");
		std::cout << "[VulkanContext::VulkanContext] INFO: pushed VAL_LAYERS_NAME\n";
	#endif


	VkResult instanceResult = vkCreateInstance(&createInfo, nullptr, &this->instance);
	
	if (instanceResult != VK_SUCCESS)
	{
		std::cerr << "[VulkanContext::VulkanContext] instance creation failed. Code " << instanceResult << "\n";
		exit(-1);
	}
	else std::cout << "Created VkInstance!\n";

	std::cout << "Creating messenger\n";

	#ifndef NDEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		debugCreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		debugCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		debugCreateInfo.pfnUserCallback = vulkanDebugCallback;
		debugCreateInfo.pUserData = nullptr;

		PFN_vkCreateDebugUtilsMessengerEXT createDebugMessenger =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT")
			);

		if (createDebugMessenger)
		{
			VkResult result = createDebugMessenger(
				instance,
				&debugCreateInfo,
				nullptr,
				&this->debugMessenger
			);

			if (result != VK_SUCCESS)
			{
				std::cerr << "Couldn't create debug messenger\n";
			} else std::cout << "Created debug messenger\n";

			
		}
	#endif

	try  
	{
		if (!pick_device())
		{
			throw std::runtime_error("[VulkanContext::VulkanContext] ERROR: Physical device not found.");
		}
		else std::cout << "[VulkanContext::VulkanContext] OK: Picked physical device\n";

		if (!create_surface(window_handle))
		{
			throw std::runtime_error("[VulkanContext::VulkanContext] ERROR: Couldn't create surface.");
		}
		else std::cout << "[VulkanContext::VulkanContext] OK: Created surface\n";

		if (!create_device(requirements))
		{
			throw std::runtime_error("[VulkanContext::VulkanContext] ERROR: Couldn't create logical device.");
		}
		else std::cout << "[VulkanContext::VulkanContext] OK: Created logical device\n";

		if (!setup_vma())
		{
			throw std::runtime_error("[VulkanContext::VulkanContext] ERROR: Couldn't setup VMA");
		} else std::cout << "[VulkanContext::VulkanContext] OK: Setup VMA\n";

		try
		{
			this->swapchain = std::make_unique<VulkanSwapchain>(*this);
			std::cout << "[VulkanContext::VulkanContext] OK: Created swapchain\n";
		}
		catch(const std::exception& e)
		{
			std::cerr << "[VulkanContext::VulkanContext] ERROR: Couldn't create swapchain:\n";
			std::cerr << e.what() << '\n';
		}


	}
	catch (const std::runtime_error& err)
	{
		std::cerr << err.what();
		exit(-1);
	}


}

void VulkanContext::shutdown()
{
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

#ifndef NDEBUG
bool VulkanContext::check_validation_layers_support()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(
        &layerCount,
        availableLayers.data()
    );

    for (const auto& layer : availableLayers)
    {
        if (strcmp(layer.layerName, VAL_LAYERS_NAME) == 0)
		{
			std::cout << "[VulkanContext::check_validation_layers_support] INFO: found VK_LAYER_KHRONOS_validation\n";
            return true;
		}
    }

	std::cout << "[VulkanContext::check_validation_layers_support] INFO: couldn't find VK_LAYER_KHRONOS_validation\n";	
    return false;
}
#endif



bool VulkanContext::pick_device()
{
	uint32_t count;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);

	std::vector<VkPhysicalDevice> pDevices(count);
	vkEnumeratePhysicalDevices(instance, &count, pDevices.data());

	std::optional<std::string> selected = std::nullopt;

	std::cout << "Found " << count << " devices: \n";
	for (int i = 0; i < count; i++)
	{
		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(pDevices[i], &prop);

		if (this->physical_device == VK_NULL_HANDLE && prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			this->physical_device = pDevices[i];
			selected = prop.deviceName;
			break;
		}
	}

	if (selected.has_value()) {
		std::cout << "Selected " << selected.value() << "\n";
	}

	if (this->physical_device == VK_NULL_HANDLE) 
	{
		if (!pDevices.empty())
		{
			std::cout << "No discrete GPU found. Falling back to first device found\n";
			this->physical_device = pDevices[0];
			return true;
		}
		else return false;
	}
	else return true;

}

/*
	Creates a surface, a connection between Vulkan and our window handle,
	the window handle can come from any window library, in fact it is a
	void pointer
*/
bool VulkanContext::create_surface(void* win_handle)
{
	VkWin32SurfaceCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.pNext = nullptr;
	info.flags = 0;
	info.hinstance = GetModuleHandle(nullptr);
	info.hwnd = (HWND)win_handle;

	VkResult creationResult = vkCreateWin32SurfaceKHR(this->instance, &info, nullptr, &this->surface);



	if (creationResult == VK_SUCCESS) return true;

	return false;
}

bool VulkanContext::create_device(ApplicationRequirements& requirements)
{
	uint32_t fam_count;
	vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &fam_count, nullptr);

	// array of single family property objects
	std::vector<VkQueueFamilyProperties> fams_props(fam_count);
	vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device, &fam_count, fams_props.data());

	std::vector<VkDeviceQueueCreateInfo> qInfos;

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
				} else std::cout << "queue family " << i << " can't do presentation\n";

				used = true;
				int count = requirements.queue_requirement(FamilyCapability::Presentation);
				if (count > q_count) q_count = count;
			}
		}
		
		
		std::vector<float> priorities;
		if (used)
		{
			priorities.resize(q_count, 1.f);

			qInfos.emplace_back();
			auto& back = qInfos.back();
			back.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			back.queueCount = q_count;
			back.queueFamilyIndex = i;
			back.pQueuePriorities = priorities.data();

			
		}

	}

	for (int i = 0; i < enum_index(FamilyCapability::Count); i++)
	{
		if (requirements.requires(index_enum<FamilyCapability>(i)) && !this->queue_families_indices[i].has_value()) 
		{
			std::cerr << "[VulkanContext::create_device] ERROR: queue families couldn't satisfy application requirements\n";
			return false;
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

	std::cout << "Creating device...\n";

	VkResult deviceResult = vkCreateDevice(this->physical_device, &info, nullptr, &this->device);
	if (deviceResult != VK_SUCCESS) 
	{
		std::cerr << "[VulkanContext::create_device] ERROR: vkCreateDevice failed\n";
		return false;
	}
	
	return true;
}

bool VulkanContext::setup_vma()
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
	if (res != VK_SUCCESS) return false;
		else return true;
}


