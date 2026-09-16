#include "Context.h"

#include "Swapchain.h"
#include "CommandManager.h"
#include "SyncManager.h"
#include "Cleaner.h"

VulkanBackend::Context::Context(void* window_handle, GSAM::Vulkan::ApplicationRequirements &requirements)
	: instance(VK_NULL_HANDLE), 
	physical_device(VK_NULL_HANDLE), 
	surface(VK_NULL_HANDLE), 
	device(VK_NULL_HANDLE), 
	queue_families_indices({std::nullopt}), 
	swapchain(nullptr),
	commandManager(nullptr),
	syncManager(nullptr),
	cleaner(std::make_unique<Cleaner>(*this))
{
	try  
	{
		create_instance();

		#ifndef NDEBUG
			create_debug_messenger();
		#endif

		pick_device();

		create_surface(window_handle);

		create_device(requirements);

		setup_vma();

		this->swapchain = std::make_unique<Swapchain>(*this);
		this->commandManager = std::make_unique<CommandManager>(this->queue_families_indices, this->device, MAX_FRAMES_IN_FLIGHT);
		this->syncManager = std::make_unique<SyncManager>(this->device, MAX_FRAMES_IN_FLIGHT);

		create_frames();

	}
	catch (const std::runtime_error& err)
	{
		std::cerr << err.what() << std::endl;
		exit(-1);
	}


}

void VulkanBackend::Context::shutdown()
{
	GSAM_VK_CHECK(vkDeviceWaitIdle(this->device), "Failed to wait for idle, something is broken");
	
	this->cleaner->FreeAssets();
	this->cleaner->FreeVulkanObjects();
	this->cleaner->FreeManagers();
	this->cleaner->FreeCore();
	this->cleaner->FreeInstance();
}

void VulkanBackend::Context::create_instance()
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

void VulkanBackend::Context::pick_device()
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
void VulkanBackend::Context::create_surface(void* win_handle)
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

void VulkanBackend::Context::create_device(GSAM::Vulkan::ApplicationRequirements& requirements)
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
		if (requirements.requires(GSAM::Vulkan::QueueFamilyCapability::Graphics))
		{
			if (!this->queue_families_indices[enum_index(GSAM::Vulkan::QueueFamilyCapability::Graphics)].has_value() && 
			fams_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				this->queue_families_indices[enum_index(GSAM::Vulkan::QueueFamilyCapability::Graphics)] = i;

				used = true;
				int count = requirements.queue_requirement(GSAM::Vulkan::QueueFamilyCapability::Presentation);
				if (count > q_count) q_count = count; 

			}
		}
		
		if (requirements.requires(GSAM::Vulkan::QueueFamilyCapability::Presentation))
		{
			if (!this->queue_families_indices[enum_index(GSAM::Vulkan::QueueFamilyCapability::Presentation)].has_value())
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
					this->queue_families_indices[enum_index(GSAM::Vulkan::QueueFamilyCapability::Presentation)] = i;	
				} 

				used = true;
				int count = requirements.queue_requirement(GSAM::Vulkan::QueueFamilyCapability::Presentation);
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

	for (int i = 0; i < enum_index(GSAM::Vulkan::QueueFamilyCapability::Count); i++)
	{
		if (requirements.requires(GSAM::Vulkan::index_enum<GSAM::Vulkan::QueueFamilyCapability>(i)) && !this->queue_families_indices[i].has_value()) 
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

void VulkanBackend::Context::create_frames()
{
	this->frames.reserve(MAX_FRAMES_IN_FLIGHT); // not resizing otherwise everything get fucked up with the indices
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		std::optional<Frame> frame = this->CreateFrame();
		this->frames.push_back(*frame);
	}
}

void VulkanBackend::Context::setup_vma()
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

TypedResourceHandle<VulkanBackend::GpuImage> VulkanBackend::Context::CreateImage(
	const VkImageCreateInfo image_create_info, 
	const VmaAllocationCreateInfo& allocation_create_info,
	VkImageViewCreateInfo& image_view_create_info
)
{
	GpuImage image = GpuImage(this->vma, image_create_info, allocation_create_info, this->device, image_view_create_info);
	auto handle = this->imageRegistry.Allocate();
	this->imageRegistry.Set(handle, image);

	this->imageHandles.push_back(handle);
	return handle;
}
TypedResourceHandle<VulkanBackend::GpuBuffer> VulkanBackend::Context::CreateBuffer(
	const VkBufferCreateInfo& buffer_create_info,
	const VmaAllocationCreateInfo& allocation_create_info
)
{
	GpuBuffer buffer(this->vma, buffer_create_info, allocation_create_info);
	auto handle = this->bufferRegistry.Allocate();
	this->bufferRegistry.Set(handle, buffer);
	return handle;
}


/*
	Temporary function, this wont be an option in GSAM which is going
	to be way lower level
*/
GSAM::GSMesh VulkanBackend::Context::CreateMesh(const std::filesystem::path& path)
{
	try
	{
		std::string ext = std::filesystem::path(path).extension().string();

		std::optional<MeshData> cpuData = std::nullopt;

		bool supported = false;
		if (ext == ".obj")
		{
			supported = true;
			cpuData = LoadMesh_Obj(path);
		} else GSAM_THROW_ERROR(ext + " isn't a supported format for meshes");

		VkDeviceSize vertBufferSize = cpuData.value().vertices.size() * sizeof(GSAM::Vertex);
		VkDeviceSize indexBufferSize = cpuData.value().indices.size() * sizeof(uint16_t);

		const VkBufferCreateInfo bufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = vertBufferSize + indexBufferSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
		};
		const VmaAllocationCreateInfo bufferAllocInfo{
			.flags = 
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
			VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |

			/*
				VMA_ALLOCATION_CREATE_MAPPED_BIT gets us a persistently mapped buffer, which in turn lets us directly copy data into VRAM
				with memcpy (How to Vulkan 2026)
			*/
			VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO
		};

		TypedResourceHandle<GpuBuffer> bufferHandle;
		
		bufferHandle = this->CreateBuffer(bufferCreateInfo, bufferAllocInfo);
		GpuBuffer* buff = this->bufferRegistry.Get(bufferHandle);
		if (!buff) GSAM_THROW_ERROR("Buffer wasn't created correctly");
		vmaSetAllocationName(this->vma, buff->allocation, "Mesh Shader GpuBuffer");

		GpuMesh mesh(*buff);
		TypedResourceHandle<GpuMesh> meshHandle = this->meshRegistry.Allocate();
		this->meshRegistry.Set(meshHandle, mesh);
		this->meshHandles.push_back(meshHandle);

		VulkanMeshImplementation m_impl 
		{
			.handle = meshHandle
		};
		
		return (GSAM::GSMesh){
			.resource_path = path.string(),
			.impl = std::make_unique<GSAM::MeshImplementation>(m_impl),
		};
		
		GSAM_LOG_DEBUG("Successfully uploaded " + path.string());

	} catch (const std::runtime_error& err)
	{
		GSAM_LOG_ERROR(err.what());
	}

	return (GSAM::GSMesh){ .resource_path = "", .impl = nullptr };
}

std::optional<VulkanBackend::Frame> VulkanBackend::Context::CreateFrame()
{
	try
	{
		uint32_t size = static_cast<uint32_t>(this->frames.size());

		Frame frame{
			.index = size,
			.commandBuffer = this->commandManager->create_command_buffer(this->device, GSAM::Vulkan::QueueFamilyCapability::Graphics),
			.shaderBuffer = ShaderBuffer(this->vma, this->device),
			
			.fence = this->syncManager->get_frame_fence(size),
			.semaphore = this->syncManager->get_frame_semaphore(size)
		};
		
		return frame;
	} catch (const std::runtime_error& err)
	{
		GSAM_LOG_ERROR(err.what());
	}
	return std::nullopt;
}

GSAM::GSTexture VulkanBackend::Context::CreateTexture(const ImageData& data)
{
	ktxTextureCreateInfo createInfo{};

    createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.baseWidth = data.width;
    createInfo.baseHeight = data.height;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;

    ktxTexture2* texture2 = nullptr;

    KTX_error_code result = ktxTexture2_Create(
        &createInfo,
        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
        &texture2
    );

	ktxTexture* texture = (ktxTexture*)(texture2);

    if (result != KTX_SUCCESS)
	{
		GSAM_LOG_ERROR("Failed to create texture (failed ktxTexture2 creation)");
        return GSAM::GSTexture();
	}

    const ktx_size_t imageSize =
        static_cast<ktx_size_t>(data.width) *
        static_cast<ktx_size_t>(data.height) *
        static_cast<ktx_size_t>(data.channels);

    result = ktxTexture_SetImageFromMemory(
        texture,
        0,                  
        0,                  
        0,                  
        data.pixels,
        imageSize
    );

    if (result != KTX_SUCCESS)
    {
        ktxTexture2_Destroy(texture2);
        return GSAM::GSTexture();
    }

	VkImageCreateInfo image_create_info{
    	.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    	.imageType = VK_IMAGE_TYPE_2D,
    	.format = ktxTexture_GetVkFormat(texture),
    	.extent = {.width = texture->baseWidth, .height = texture->baseHeight, .depth = 1 },
    	.mipLevels = texture->numLevels,
    	.arrayLayers = 1,
    	.samples = VK_SAMPLE_COUNT_1_BIT,
    	.tiling = VK_IMAGE_TILING_OPTIMAL,
    	.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // disk to image
    	.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VmaAllocationCreateInfo image_allocation_create_info{ .usage = VMA_MEMORY_USAGE_AUTO };

	VkImageViewCreateInfo image_view_create_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = ktxTexture_GetVkFormat(texture),
		.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = texture->numLevels, .layerCount = 1 }
	};

	try 
	{
		TypedResourceHandle<GpuImage> handle = this->CreateImage(image_create_info, image_allocation_create_info, image_view_create_info);
		VulkanTextureImplementation impl {
			.handle = handle,
			.ktx = texture
		};

		GSAM::GSTexture out = {
			.impl = std::make_unique<GSAM::TextureImplementation>(impl)
		};

		return out;
	} catch (const std::runtime_error& err)
	{
		GSAM_LOG_ERROR("Failed to create texture (Failed to create texture image: " + std::string(err.what()) + ")");
		return GSAM::GSTexture();
	}	


}

