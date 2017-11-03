#ifndef __TFTJ_LINEAR_ALLOCATOR__
#define __TFTJ_LINEAR_ALLOCATOR__

#include <memory>

struct LinearAllocator
{
private:
	const int capacity;

	char* start_memory;
	char* current_memory;

	int index;

public:
	LinearAllocator(int capacity_bytes) :
		capacity(capacity_bytes),
		start_memory(reinterpret_cast<char*>(malloc(capacity_bytes)))
	{
		current_memory = start_memory;
		index = 0;
	}

	LinearAllocator(LinearAllocator&& ref, int capacity_bytes) :
		capacity(capacity_bytes),
		start_memory(reinterpret_cast<char*>(realloc(ref.start_memory, capacity_bytes)))
	{
		current_memory = start_memory;
		index = 0;
		ref.start_memory = nullptr;
	}

	void* allocate_raw(int nBytes)
	{
		index += nBytes;
		if (index <= capacity)
		{
			char* new_memory = current_memory;
			current_memory += nBytes;
			//test
			//memset(new_memory, 0, nBytes);
			return new_memory;
		}
		else
		{
			return nullptr;
		}
	}

	template<class T>
	T* allocate(int size)
	{
		return reinterpret_cast<T*>(allocate_raw(sizeof(T)*size));
	}

	void deallocate_raw(int nBytes)
	{
		index -= nBytes;
		current_memory -= nBytes;
	}

	template<class T>
	void deallocate(int size)
	{
		deallocate_raw(sizeof(T)*size);
	}

	~LinearAllocator()
	{
		free(start_memory);
		//TODO assert index == 0
	}

	LinearAllocator(const LinearAllocator&) = delete;
	LinearAllocator(const LinearAllocator&&) = delete;
	LinearAllocator& operator=(const LinearAllocator&) = delete;
	LinearAllocator& operator=(const LinearAllocator&&) = delete;
};

#endif