#pragma once

/*
	needed to get the VK_KHR_win32_Surface extension, in GSAM we're going to 
	select the right one based on the user's OS, im on Windows so WIN32
*/
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#include <array>
#include <utility>

enum class FamilyCapability
{
    Graphics = 0,
    Presentation,
    
    Count
};

constexpr inline size_t capability_count() { return static_cast<size_t>(FamilyCapability::Count); }

class FamilyQueueRequirements
{
private:
    // requires, queue requirement
   std::array<std::pair<bool, int>, capability_count()> requirements;

public:
    bool requires(FamilyCapability capability) const;
    int queue_requirement(FamilyCapability capability) const;
    void set_requirement(FamilyCapability capability, bool value, int queue_requirement);
};

template <typename E>
constexpr inline int enum_index(E Enum) { return static_cast<int>(Enum); }

template <typename E>
constexpr inline E index_enum(int index) { return static_cast<E>(index); }

#include <iostream>
#include <vulkan/vulkan.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    // 1. Filter out VERBOSE messages to prevent massive console spam on startup
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        return VK_FALSE; 
    }

    // 2. Determine human-readable strings for Severity and Type
    const char* severityStr = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" :
                              (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARNING" : "INFO";
                              
    const char* typeStr = (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? "Validation" :
                          (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ? "Performance" : "General";

    // 3. Print the formatted message, including the VUID (pMessageIdName)
    std::cerr << "[Vulkan " << severityStr << " | " << typeStr << "] " 
              << pCallbackData->pMessageIdName << "\n"
              << "  -> " << pCallbackData->pMessage << "\n\n";

    // 4. (Optional) Trigger a debugger breakpoint on critical errors
    // This is incredibly useful for catching the exact line of code that caused the error.
    /*
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        #ifdef _WIN32
            __debugbreak();
        #else
            __builtin_trap();
        #endif
    }
    */

    // VK_FALSE tells Vulkan that we do NOT want to abort the application/call
    return VK_FALSE; 
}

