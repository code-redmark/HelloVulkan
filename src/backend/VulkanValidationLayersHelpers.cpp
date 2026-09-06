/*
    Contains all functions related to the creation and the usage of validation layers
*/

#include "VulkanContext.h"

#ifndef NDEBUG

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        return VK_FALSE; 
    }
    const char* severityStr = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" :
                              (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARNING" : "INFO";
                              
    const char* typeStr = (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? "Validation" :
                          (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ? "Performance" : "General";

    const char* id_name = pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "";

    std::cerr << "[Vulkan " << severityStr << " | " << typeStr << "] " 
              << id_name << "\n"
              << "  -> " << pCallbackData->pMessage << "\n\n";

    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        #ifdef _WIN32
            __debugbreak();
        #else
            __builtin_trap();
        #endif
    }

    return VK_FALSE; 
}

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
        if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
		{
			GSAM_LOG_DEBUG("found VK_LAYER_KHRONOS_validation");
            return true;
		}
    }

	std::cout << "[VulkanContext::check_validation_layers_support] INFO: couldn't find VK_LAYER_KHRONOS_validation\n";	
    return false;
}

bool VulkanContext::create_debug_messenger() {

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

        if (result != VK_SUCCESS) return false;
            else return true;        
    } else return false;
}

#endif