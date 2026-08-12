
/*
	needed to get the VK_KHR_win32_Surface extension, in GSAM we're going to 
	select the right one based on the user's OS, im on Windows so WIN32
*/

#include "VulkanContext.h"
#include <iostream>

#include <optional>



VulkanContext::VulkanContext()
{
	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	VkApplicationInfo appInfo{};
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.applicationVersion = 0;
	appInfo.engineVersion = 0;
	appInfo.pApplicationName = nullptr;
	appInfo.pEngineName = nullptr;
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.enabledExtensionCount = 2;
	instanceInfo.ppEnabledExtensionNames = extensions;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;

	VkResult instanceResult = vkCreateInstance(&instanceInfo, nullptr, &this->instance);
	
	if (instanceResult != VK_SUCCESS)
	{
		throw std::runtime_error("VulkanContext instance creation failed.");
	}
	else std::cout << "Created VkInstance!\n";

}

void VulkanContext::Init(void* window_handle, FamilyQueueRequirements &requirements)
{
	try  
	{
		if (!pick_device())
		{
			throw std::runtime_error("[VulkanContext::Init] ERROR: physical device not found.");
		}
		else std::cout << "[VulkanContext::Init] OK: Found device\n";

		if (!create_surface(window_handle))
		{
			throw std::runtime_error("[VulkanContext::Init] ERROR: couldn't create surface.");
		}
		else std::cout << "[VulkanContext::Init] OK: Created surface\n";

		if (!create_device(requirements))
		{
			throw std::runtime_error("[VulkanContext::Init] ERROR: couldn't create logical device.");
		}
		else std::cout << "[VulkanContext::Init] OK: Created logical device\n";

		this->swapchain = std::unique_ptr<VulkanSwapchain>(new VulkanSwapchain(this->device, this->physical_device, this->surface, this->queue_families_indices));
		if (this->swapchain == nullptr)
		{
			throw std::runtime_error("[VulkanContext::Init] ERROR: couldn't create swapchain.");
		}
		else std::cout << "[VulkanContext::Init] OK: Created swapchain\n";

	}
	catch (std::runtime_error err)
	{
		std::cout << err.what();
		exit(-1);
	}
	
}

void VulkanContext::shutdown()
{
	if (this->instance != VK_NULL_HANDLE) {
		vkDestroyInstance(this->instance, nullptr);
		this->instance = VK_NULL_HANDLE;
	}
}

/*
	Gets all the physical devices vulkan recognized and selects the first
	discrete GPU found, falls back to the first GPU found if no GPU is found
	and throws an error if there's no GPU
*/
bool VulkanContext::pick_device()
{
	uint32_t count;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);

	std::vector<VkPhysicalDevice> pDevices(count);
	vkEnumeratePhysicalDevices(instance, &count, pDevices.data());

	std::string selected;

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

	std::cout << "Selected " << selected << "\n";

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

bool VulkanContext::create_device(FamilyQueueRequirements& requirements)
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
		

		if (used)
		{
			std::vector<float> priorities(q_count, 1.f);

			qInfos.emplace_back();
			auto& back = qInfos.back();
			back.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			back.queueCount = q_count;
			back.queueFamilyIndex = i;
			back.pQueuePriorities = priorities.data();

			
		}

	}

	// Check if requirements were satisfied
	for (int i = 0; i < enum_index(FamilyCapability::Count); i++)
	{
		if (requirements.requires(index_enum<FamilyCapability>(i)) && !this->queue_families_indices[i].has_value()) 
		{
			std::cerr << "[VulkanContext::create_device] ERROR: queue families couldn't satisfy application requirements\n";
			return false;
		}
	}


	VkDeviceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Queue info
	info.queueCreateInfoCount = qInfos.size();
	info.pQueueCreateInfos = qInfos.data();

	// swapchain 
	const char* names[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	info.ppEnabledExtensionNames = names;
	info.enabledExtensionCount = 1;

	info.pEnabledFeatures = nullptr;
	info.flags = 0;
	info.enabledLayerCount = 0;

	VkResult deviceResult = vkCreateDevice(this->physical_device, &info, nullptr, &this->device);
	if (deviceResult != VK_SUCCESS) 
	{
		std::cerr << "[VulkanContext::create_device] ERROR: vkCreateDevice failed\n";
		return false;
	}
	
	return true;
}

bool VulkanContext::create_swapchain()
{

	

	return true;
}
