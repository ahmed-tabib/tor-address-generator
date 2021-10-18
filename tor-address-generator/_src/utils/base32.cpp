#include "base32.h"

static void base32_pack(const char* in_8, char* out_5)
{
	uint64_t buffer = 0;

	for (size_t i = 0; i < 8; i++)
	{
		buffer |= in_8[i];
		buffer <<= 5;
	}
	buffer >>= 5;

	for (int i = 4; i >= 0; i--)
		out_5[4 - i] = ((char*)&buffer)[i];
}

void base32_decode(std::string in, char** &out, size_t& out_size, size_t& out_count)
{
	const char base32_charset[32] =
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
		'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
		'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
		'y', 'z', '2', '3', '4', '5', '6', '7'
	};

	size_t bloated_buffer_size = in.length() + (8 - (in.length() % 8));
	char* bloated_buffer = new char[bloated_buffer_size];
	memset(bloated_buffer, 0, bloated_buffer_size);

	for (size_t i = 0; i < in.length(); i++)
	{
		for (size_t j = 0; j < 32; j++)
		{
			if (in[i] == base32_charset[j])
			{
				bloated_buffer[i] = j;
				break;
			}
		}
	}

	size_t packed_buffer_size = (bloated_buffer_size / 8) * 5;
	char* packed_buffer = new char[packed_buffer_size];
	memset(packed_buffer, 0, packed_buffer_size);

	for (size_t i = 0; i < bloated_buffer_size / 8; i++)
	{
		base32_pack(bloated_buffer + i * 8, packed_buffer + i * 5);
	}

	size_t filler_bits = 8 - ((in.length() * 5) % 8);
	size_t required_byte_count = ((in.length() * 5) + filler_bits) / 8;
	char* required_buffer = new char[required_byte_count];
	memcpy(required_buffer, packed_buffer, required_byte_count);

	size_t possible_decode_count = 1;
	for (size_t i = 0; i < filler_bits; i++)
		possible_decode_count *= 2;

	out = new char*[possible_decode_count];

	for (unsigned char i = 0; i < possible_decode_count; i++)
	{
		out[i] = new char[required_byte_count];
		memcpy(out[i], required_buffer, required_byte_count);

		out[i][required_byte_count - 1] |= i;
	}

	out_size = required_byte_count;
	out_count = possible_decode_count;

	delete[] required_buffer;
	delete[] packed_buffer;
	delete[] bloated_buffer;
}