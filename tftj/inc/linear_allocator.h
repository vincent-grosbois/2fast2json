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
			//TODO throw
			return nullptr;
		}
	}

	template<class T>
	T* allocate(int count)
	{
		return reinterpret_cast<T*>(allocate_raw(sizeof(T)*count));
	}

	void deallocate_raw(int nBytes)
	{
		index -= nBytes;
		current_memory -= nBytes;
	}

	template<class T>
	void deallocate(int count)
	{
		deallocate_raw(sizeof(T)*count);
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

template<class T>
struct ScopedAllocated
{
	LinearAllocator& alloc;
	const int size;

	T* allocated;

	ScopedAllocated<T>(LinearAllocator& alloc, int size):
		alloc(alloc),
		size(size)
	{
		allocated = alloc.allocate<T>(size);
	}

	~ScopedAllocated()
	{
		alloc.deallocate<T>(size);
	}

	ScopedAllocated(const ScopedAllocated<T>&) = delete;
	ScopedAllocated(const ScopedAllocated<T>&&) = delete;
	ScopedAllocated& operator=(const ScopedAllocated<T>&) = delete;
	ScopedAllocated& operator=(const ScopedAllocated<T>&&) = delete;
};

#endif