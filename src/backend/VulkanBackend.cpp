#include "VulkanBackend.h"

void VulkanBackend::Buffer::Free(VmaAllocator allocator)
{
	vmaDestroyBuffer(allocator, this->buffer, this->allocation);
}

VulkanBackend::ShaderBuffer::ShaderBuffer(VmaAllocator allocator, const VkDevice& device)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(ShaderData);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VkResult res = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &this->buffer, &this->allocation, &this->allocationInfo);
	vmaSetAllocationName(allocator, this->allocation, "ShaderBuffer");

	VkBufferDeviceAddressInfo deviceAddressInfo{};
	deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	deviceAddressInfo.buffer = this->buffer;
	this->deviceAddress = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
}

bool GSAM::Vulkan::ApplicationRequirements::requires(GSAM::Vulkan::QueueFamilyCapability capability) const
{
	return requirements[static_cast<int>(capability)].first;
}

void GSAM::Vulkan::ApplicationRequirements::set_requirement(GSAM::Vulkan::QueueFamilyCapability capability, bool value, int queue_requirement)
{
	requirements[static_cast<int>(capability)] = std::pair<bool, int>(value, queue_requirement);

}

int GSAM::Vulkan::ApplicationRequirements::queue_requirement(GSAM::Vulkan::QueueFamilyCapability capability) const
{
	return requirements[static_cast<int>(capability)].second; 
}

void VulkanBackend::GpuMesh::Free(VmaAllocator vma)
{
	vmaDestroyBuffer(vma, this->buffer, this->allocation);
}

void VulkanBackend::Frame::Free(VmaAllocator vma)
{
	this->shaderBuffer.Free(vma);
}

VulkanBackend::ImageData VulkanBackend::load_image(const std::filesystem::path& path)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels)
	{
		GSAM_LOG_ERROR("Failed to load image: " + path.string());
	}

	ImageData imageData{};
	imageData.width = width;
	imageData.height = height;
	imageData.channels = 4; // STBI_rgb_alpha forces 4 channels
	imageData.pixels = pixels;

	return imageData;
}

VulkanBackend::MeshData VulkanBackend::LoadMesh_Obj(const std::filesystem::path& path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	std::ifstream stream(path);
	if (!stream.is_open()) GSAM_THROW_ERROR("Couldn't open " + path.string());

	if (tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &stream, nullptr, true) != true)
		GSAM_THROW_ERROR("Couldn't load obj " + path.string() + "\ntinyobj error: " + err);

	stream.close();

	MeshData data;

	const VkDeviceSize indexCount = shapes[0].mesh.indices.size();
	for (const auto& index : shapes[0].mesh.indices)
	{
		GSAM::Vertex v{
			.pos = {
				attrib.vertices[index.vertex_index * 3], 
				-attrib.vertices[index.vertex_index * 3 + 1], 
				attrib.vertices[index.vertex_index * 3 + 2] 
			}
		};
		if (index.normal_index >= 0)
		{
			v.normal = {
				attrib.normals[index.normal_index * 3],
				-attrib.normals[index.normal_index * 3 + 1],
				attrib.normals[index.normal_index * 3 + 2]
			};
		}
		if (index.texcoord_index >= 0)
		{
			v.uv = {
				attrib.texcoords[index.texcoord_index * 2],
				1.0 - attrib.texcoords[index.texcoord_index * 2 + 1]
			};
		}


    	data.vertices.push_back(v);
    	data.indices.push_back(data.indices.size());
	}

	return data;
}