#pragma once

#include "Core/Core.h"

namespace Fluent
{

	class StackAllocator
	{
	public:

		explicit StackAllocator();
		explicit StackAllocator(u32 stackLimit);

		StackAllocator(const StackAllocator& other) = delete;
		StackAllocator(StackAllocator&& other) = delete;
		StackAllocator& operator=(const StackAllocator& other) = delete;

		~StackAllocator();

	public:

		template <typename T, typename ... Args>
		friend T* NewStack(StackAllocator& memStack, Args... args);

		template <typename T>
		friend void DeleteStack(StackAllocator& memStack, T* object);

		template <typename T>
		friend T* NewStackByArray(StackAllocator& memStack, i32 size);

		template <typename T>
		friend void DeleteStackByArray(StackAllocator& memStack, T* object, i32 size);

	private:

		u8* mMemoryStack;
		u8* mStackBottom;
		u8* mStackTop;

		u32 mStackLimit;

	};


	template <typename T, typename ... Args>
	T* NewStack(StackAllocator& memStack, Args... args)
	{
		const u32 objectSize = sizeof(T);
		if (memStack.mStackTop < memStack.mStackBottom + objectSize)
		{
			return nullptr;
		}

		T newObject(args...);

		u8* convertPtr = reinterpret_cast<u8*>(&newObject);
		for (u32 index = 0; index < objectSize; ++index)
		{
			memStack.mStackBottom[index] = convertPtr[index];
		}

		u8* stackBottomCache = memStack.mStackBottom;
		memStack.mStackBottom += objectSize;

		return reinterpret_cast<T*>(stackBottomCache);
	}

	template <typename T>
	void DeleteStack(StackAllocator& memStack, T* object)
	{
		if (!object)
		{
			return;
		}

		const u32 objectSize = sizeof(T);
		u8* convertPtr = reinterpret_cast<u8*>(object);
		if (convertPtr == memStack.mStackBottom - objectSize)
		{
			memStack.mStackBottom -= objectSize;
			for (u32 index = 0; index < objectSize; ++index)
			{
				memStack.mStackBottom[index] = 0;
			}
		}
		else
		{
			Assert(false);
		}
	}

	template <typename T>
	T* NewStackByArray(StackAllocator& memStack, i32 size)
	{
		const u32 objectSize = sizeof(T);
		if (memStack.mStackTop < memStack.mStackBottom + (objectSize * size))
		{
			return nullptr;
		}

		u8* stackBottomCache = memStack.mStackBottom;
		for (i32 arrayIndex = 0; arrayIndex < size; ++arrayIndex)
		{
			T newObject = T();

			u8* convertPtr = reinterpret_cast<u8*>(&newObject);
			for (u32 index = 0; index < objectSize; ++index)
			{
				memStack.mStackBottom[index] = convertPtr[index];
			}

			memStack.mStackBottom += objectSize;
		}

		return reinterpret_cast<T*>(stackBottomCache);
	}

	template <typename T>
	void DeleteStackByArray(StackAllocator& memStack, T* object, i32 size)
	{
		if (!object)
		{
			return;
		}

		const u32 objectSize = sizeof(T);
		u8* convertPtr = reinterpret_cast<u8*>(object);
		if (convertPtr == memStack.mStackBottom - (objectSize * size))
		{
			memStack.mStackBottom -= objectSize * size;
			const u32 indexEnd = objectSize * size;
			for (u32 index = 0; index < indexEnd; ++index)
			{
				memStack.mStackBottom[index] = 0;
			}
		}
		else
		{
			Assert(false);
		}
	}

}

