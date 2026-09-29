#include "VulkanBackend.h"





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


void VulkanBackend::Frame::Free(VmaAllocator vma)
{
	this->shaderBuffer.Free(vma);
}

VulkanBackend::ImageData VulkanBackend::load_image(const std::filesystem::path& path)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) GSAM_LOG_ERROR("Failed to load image: " + path.string());

	ImageData imageData{};
	imageData.width = width;
	imageData.height = height;
	imageData.channels = 4; // STBI_rgb_alpha forces 4 channels
	imageData.pixels = pixels;

	return imageData;
}

VulkanBackend::GpuImage::GpuImage(
	const VmaAllocator& vma, 
	const VkImageCreateInfo& image_create_info, 
	const VmaAllocationCreateInfo& allocation_create_info,
	const VkDevice& device,
	VkImageViewCreateInfo& image_view_create_info
)
{
	VkResult res = vmaCreateImage(vma, &image_create_info, &allocation_create_info, &this->image, &this->allocation, nullptr);
	GSAM_VK_CHECK(res, "Failed to create GpuImage (Failed to create VkImage)");

	image_view_create_info.image = this->image;
	VkResult view_res = vkCreateImageView(device, &image_view_create_info, nullptr, &this->imageView);

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