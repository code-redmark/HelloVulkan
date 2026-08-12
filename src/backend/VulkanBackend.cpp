#include "VulkanBackend.h"

bool FamilyQueueRequirements::requires(FamilyCapability capability) const
{
	return requirements[static_cast<int>(capability)].first;
}

void FamilyQueueRequirements::set_requirement(FamilyCapability capability, bool value, int queue_requirement)
{
	requirements[static_cast<int>(capability)] = std::pair<bool, int>(value, queue_requirement);

}

int FamilyQueueRequirements::queue_requirement(FamilyCapability capability) const
{
	return requirements[static_cast<int>(capability)].second; 
}