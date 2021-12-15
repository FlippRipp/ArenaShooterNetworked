#include "BinaryStream.h"

BinaryWriter::~BinaryWriter()
{
	if (buffer)
		free(buffer);
}