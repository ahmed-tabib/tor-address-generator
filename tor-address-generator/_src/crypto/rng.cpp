#include "rng.h"

unsigned char gen_rand_char()
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> char_dist(0x00, 0xff);
	
	return (unsigned char)char_dist(rng);
}

void gen_rand_buffer(unsigned char* buffer, size_t byte_count)
{
	for (size_t i = 0; i < byte_count; i++)
	{
		buffer[i] = gen_rand_char();
	}
}