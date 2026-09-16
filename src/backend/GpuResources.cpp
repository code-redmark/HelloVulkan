#include "GpuResources.hpp"

#include "VulkanBackend.h"

VulkanBackend::GpuBuffer::GpuBuffer(
    VmaAllocator& vma, 
    const VkBufferCreateInfo& buffer_create_info, 
    const VmaAllocationCreateInfo& allocation_create_info
)
{
    GSAM_VK_CHECK(
        vmaCreateBuffer(vma, &buffer_create_info, &allocation_create_info, &this->buffer, &this->allocation, &this->allocationInfo),
        "Couldn't create buffer"
    );
}

void VulkanBackend::GpuBuffer::Free(const VmaAllocator& allocator)
{
	vmaDestroyBuffer(allocator, this->buffer, this->allocation);
}


VulkanBackend::ShaderBuffer::ShaderBuffer(const VmaAllocator &vma, const VkDevice& device)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(ShaderData);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VkResult res = vmaCreateBuffer(vma, &bufferCreateInfo, &allocInfo, &this->buffer, &this->allocation, &this->allocationInfo);
	vmaSetAllocationName(vma, this->allocation, "ShaderBuffer");

	VkBufferDeviceAddressInfo deviceAddressInfo{};
	deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	deviceAddressInfo.buffer = this->buffer;
	this->deviceAddress = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
}

VulkanBackend::GpuMesh::GpuMesh(const GpuBuffer& buffer)
    : buffer(buffer) {}