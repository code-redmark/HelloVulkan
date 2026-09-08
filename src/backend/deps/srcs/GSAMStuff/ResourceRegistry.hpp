#pragma once

#include "Types.hpp"

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <cstring>

class GSContext;

struct ResourceSlot
{
	uint32_t generation = 1;
	bool isAlive = true;

	/*
		Alignment is for memory safety, for
		SIMD instructions and for performance
	*/
	alignas(16) uint8_t slotData[32] = {0};
};

constexpr ResourceSlot nullslot = { 0, false, {0}};

class ResourceRegistry
{
private:
	
	std::vector<ResourceSlot> resource_pool{0};
	std::vector<uint64_t> free_pool;

public:
	ResourceRegistry();

	ResourceHandle Allocate();

	void Release(const ResourceHandle& handle);

	void* Access(const ResourceHandle& handle);

};