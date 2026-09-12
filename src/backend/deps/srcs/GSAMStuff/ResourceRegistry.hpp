#pragma once

#include "Types.hpp"

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <cstring>
#include <optional>


struct ResourceSlot
{
	uint32_t generation = 1;
	bool isAlive = true;

	static constexpr std::size_t Capacity = 64;
	static constexpr std::size_t Alignment = 16;

	/*
		Alignment is for memory safety, for
		SIMD instructions and for performance
	*/
	alignas(Alignment) uint8_t slotData[Capacity] = {0};
};

constexpr ResourceSlot nullslot = { 0, false, {0}};

class ResourceRegistry
{
private:
	
	std::vector<ResourceSlot> resource_pool{0};
	std::vector<uint64_t> free_pool;

	void* Access(const ResourceHandle& handle);

public:
	ResourceRegistry();

	ResourceHandle Allocate();

	void Release(const ResourceHandle& handle);

	template<typename T>
	T* Get(const ResourceHandle& handle)
	{
		void* memory = this->Access(handle);
		return static_cast<T*>(memory);
	}

	template<typename T>
	void Set(const ResourceHandle& handle, const T& data)
	{
		static_assert(sizeof(T) <= ResourceSlot::Capacity, "Data type is too large to store in ResourceSlot");
		static_assert(alignof(T) <= ResourceSlot::Alignment, "Data type alignment is too large to store in ResourceSlot");
		static_assert(std::is_trivially_copyable<T>::value, "Data type must be trivially copyable");

		this->Access(handle);
		memcpy(this->resource_pool[handle.id].slotData, &data, sizeof(T));
	}

};

