#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <cstring>
#include <optional>

#include "Log.hpp"

template <typename T>
struct TypedResourceHandle
{
    uint32_t id;
    uint64_t generation;
};

template <typename T> 
struct TypedResourceSlot
{
	uint64_t generation = 1;
	bool isAlive = true;

	std::optional<T> data = std::nullopt;
};

template <typename T>
class TypedResourceRegistry
{
private:
    bool is_handle_valid(const TypedResourceHandle<T>& handle)
    {
        if (handle.id == 0) 
        {
            GSAM_LOG_ERROR("Passed handle is invalid (nullslot ID)");
            return false;
        }
        if (handle.id >= this->resource_pool.size()) 
        {
            GSAM_LOG_ERROR("Passed handle is invalid (handle.id >= resource_pool.size())");
            return false;
        }
        if (handle.generation != this->resource_pool[handle.id].generation) 
        {
            GSAM_LOG_ERROR("Passed handle is invalid (handle.generation != resource_pool[handle.id].generation)");
            return false;
        }
        if (this->resource_pool[handle.id].isAlive == false)
        {
            GSAM_LOG_ERROR("Passed handle is invalid (resource_pool[handle.id].isAlive is false)");
            return false;
        }

        return true;
    }

	std::vector<TypedResourceSlot<T>> resource_pool{};
	std::vector<uint32_t> free_pool;

public:
	TypedResourceRegistry()
    {
        TypedResourceSlot<T> nullslot;
        nullslot.data = std::nullopt;
        nullslot.generation = 0;
        nullslot.isAlive = false;
        this->resource_pool.push_back(nullslot);

        this->free_pool = {};
    }

	TypedResourceHandle<T> Allocate()
    {
        TypedResourceHandle<T> handle;
        if (this->free_pool.size() == 0)
        {
            this->resource_pool.emplace_back();
            handle.id = this->resource_pool.size() - 1;
            handle.generation = 1;
        }
        else 
        {
            this->resource_pool[this->free_pool.back()].isAlive = true;
            handle.generation = this->resource_pool[this->free_pool.back()].generation;
            handle.id = this->free_pool.back();

            this->free_pool.pop_back();
        }

        return handle;
    }

	void Release(const TypedResourceHandle<T>& handle)
    {
        if (!this->is_handle_valid(handle)) return;

        TypedResourceSlot<T>& slot = this->resource_pool[handle.id];
        
        slot.generation++;
        slot.isAlive = false;
        slot.data.reset();

        this->free_pool.push_back(handle.id);
    }

	std::optional<T>* Get(const TypedResourceHandle<T>& handle)
	{
        if (!this->is_handle_valid(handle)) return nullptr;

        return &this->resource_pool[handle.id].data;
	}

	void Set(const TypedResourceHandle<T>& handle, const T& data)
	{
		std::optional<T>* p_slot_data = this->Get(handle);
        if (p_slot_data != nullptr)
        {
            *p_slot_data = data;
        } else GSAM_LOG_ERROR("Couldn't set slot data because the passed handle is invalid");
	}
};