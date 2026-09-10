#include "VulkanBackend.h"

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VkBufferCreateInfo bufferCreateInfo, VmaAllocationCreateInfo bufferAllocInfo)
{
	GSAM_VK_CHECK(vmaCreateBuffer(allocator, &bufferCreateInfo, &bufferAllocInfo, &this->buffer, &this->allocation, &this->allocationInfo), "Failed to create buffer");
}

ShaderDataBuffer::ShaderDataBuffer(VmaAllocator allocator, const VkDevice& device)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(ShaderData);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	this->buffer = VulkanBuffer(allocator, bufferCreateInfo, allocInfo);

	VkBufferDeviceAddressInfo deviceAddressInfo{};
	deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	deviceAddressInfo.buffer = this->buffer.buffer;
	this->deviceAddress = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
}

void VulkanFrame::Free(VmaAllocator vma)
{
	vmaDestroyBuffer(vma, this->shaderBuffer.buffer.buffer, this->shaderBuffer.buffer.allocation);
}

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

void VulkanGpuMesh::destroyBuffer(VmaAllocator vma)
{
	vmaDestroyBuffer(vma, this->buffer, this->allocation);
}