#include "VulkanBackend.h"

bool ApplicationRequirements::requires(FamilyCapability capability) const
{
	return requirements[static_cast<int>(capability)].first;
}

void ApplicationRequirements::set_requirement(FamilyCapability capability, bool value, int queue_requirement)
{
	requirements[static_cast<int>(capability)] = std::pair<bool, int>(value, queue_requirement);

}

int ApplicationRequirements::queue_requirement(FamilyCapability capability) const
{
	return requirements[static_cast<int>(capability)].second; 
}

