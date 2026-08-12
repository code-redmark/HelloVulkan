#pragma once

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

