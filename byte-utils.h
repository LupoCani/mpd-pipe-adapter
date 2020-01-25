#pragma once
#include <stdint.h>
#include <stdio.h>

#define BYTE_MAX UINT8_MAX
typedef uint8_t byte;

typedef uint8_t bool;
const bool false = 0;
const bool true = 1;

#define nullptr NULL

byte clamp_to_byte(uint32_t a)
{
	if (a > BYTE_MAX)
		return BYTE_MAX;
		
	return a % BYTE_MAX;
}

void uint32_to_bytes(uint32_t a, byte* bytes)
{
	for (int i = 0; i < 4; ++i)
	{
		bytes[i] = a % BYTE_MAX;
		a /= BYTE_MAX;
	}
}

void bytes_to_int(uint32_t *a, byte* bytes)
{
	*a = 0;
	uint32_t pos = 1;
	for (int i = 0; i < 4; ++i)
	{
		(*a) += (bytes[i] * pos);
		pos *= BYTE_MAX;
	}
}
