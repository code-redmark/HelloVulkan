#pragma once

/*
	needed to get the VK_KHR_win32_Surface extension, in GSAM we're going to 
	select the right one based on the user's OS, im on Windows so WIN32
*/
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include "deps/vma.h"

#include <array>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
    #define GSAM_FUNC_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define GSAM_FUNC_NAME __FUNCSIG__
#else
    #define GSAM_FUNC_NAME __func__ // Fallback
#endif

#define GSAM_LOG_INFO(msg) std::cout << "[" << GSAM_FUNC_NAME << "] INFO: " << msg << "\n"

#ifndef NDEBUG
    #define GSAM_LOG_DEBUG(msg) \
        std::cout << "[" << GSAM_FUNC_NAME << "] DEBUG: " << msg << "\n"
#else
    #define GSAM_LOG_DEBUG(msg) \
        do {} while(0)
#endif

#define GSAM_THROW_ERROR(msg) throw std::runtime_error(std::string("[") + GSAM_FUNC_NAME +  std::string("] ERROR: ") + std::string(msg))

#define GSAM_VK_CHECK(result, msg) if (result != VK_SUCCESS) GSAM_THROW_ERROR(msg);

enum class FamilyCapability
{
    Graphics = 0,
    Presentation,
    
    Count
};

constexpr inline size_t capability_count() { return static_cast<size_t>(FamilyCapability::Count); }

class ApplicationRequirements
{
private:
    // requires, amount of queues required
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


