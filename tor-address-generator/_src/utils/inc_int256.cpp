#include "incr_int256.h"

void increment_int256(unsigned char big_int[32])
{
	for (int i = 31; i >= 0; i--)
	{
		big_int[i]++;

		if (big_int[i] != 0x00)
			break;
	}
}