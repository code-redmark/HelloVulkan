#include "ResourceRegistry.hpp"

ResourceRegistry::ResourceRegistry()
{
	this->resource_pool.push_back(nullslot);
	this->free_pool = {};
}

ResourceHandle ResourceRegistry::Allocate()
{
	ResourceHandle handle;
	if (this->free_pool.size() == 0)
	{
		this->resource_pool.emplace_back();
		handle.id = this->resource_pool.size() - 1;
		handle.generation = 1;
	}
	else 
	{
		std::cout << free_pool[0];
		this->resource_pool[this->free_pool.back()].isAlive = true;
		handle.generation = this->resource_pool[this->free_pool.back()].generation;
		handle.id = this->free_pool.back();

		this->free_pool.pop_back();
	}

	return handle;
}

void ResourceRegistry::Release(const ResourceHandle& handle)
{
	if (handle.id == 0) return;
	if (handle.id >= this->resource_pool.size()) return;
	if (handle.generation != this->resource_pool[handle.id].generation) return;
	if (this->resource_pool[handle.id].isAlive == false) return;

	this->resource_pool[handle.id].generation++;
	this->resource_pool[handle.id].isAlive = false;
	std::memset(this->resource_pool[handle.id].slotData, 0, sizeof(ResourceSlot::slotData));

	this->free_pool.push_back(handle.id);

}

void* ResourceRegistry::Access(const ResourceHandle& handle)
{
	if (handle.id == 0) return nullptr;
	if (handle.id >= this->resource_pool.size()) return nullptr;
	if (handle.generation != this->resource_pool[handle.id].generation) return nullptr;
	if (this->resource_pool[handle.id].isAlive == false) return nullptr;

	// if something goes bad put back those ifs here and look at it again
	return this->resource_pool[handle.id].slotData;
}

