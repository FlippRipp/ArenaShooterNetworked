#pragma once
#include <stdlib.h>
#include <string.h>
#include "Engine.h"

struct BinaryWriter
{
public:
	~BinaryWriter();

	template<typename T>
	void write(T value)
	{
		ensureCapacity(offset + sizeof(T));
		memcpy(buffer + offset, &value, sizeof(T));
		offset += sizeof(T);
	}

	char* ptr() { return buffer; }
	int size() { return offset; }

private:
	void ensureCapacity(int newCapacity)
	{
		if (capacity > newCapacity)
			return;

		char* newBuffer = (char*)malloc(newCapacity);
		if (buffer)
		{
			memcpy(newBuffer, buffer, offset);
			free(buffer);
		}

		buffer = newBuffer;
		capacity = newCapacity;
	}

	char* buffer = nullptr;
	int capacity = 0;
	int offset = 0;
};

struct BinaryReader
{
	BinaryReader(const char* data, int size)
		:data(data), size(size)
	{
	}

	template<typename T>
	T read()
	{
		if (offset + sizeof(T) > size)
		{
			engError("BinaryReader reading out of bounds");
			return T();
		}

		T value;
		memcpy(&value, data + offset, sizeof(T));
		offset += sizeof(T);

		return value;
	}

private:
	const char* data;
	int size;
	int offset = 0;
};