#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>

#include "utils/base32.h"
#include "utils/incr_int256.h"

#include "crypto/rng.h"
#define COMPACT_DISABLE_X25519
#include "crypto/ext/compact25519.h"

//Global variables
size_t match_spec_count = 0;
size_t match_spec_size = 0;
char** match_specs = nullptr;
bool match_found = false;
char match_seed[32];

void thread_main()
{
	unsigned char seed[32];
	unsigned char public_key[32];
	unsigned char private_key[64];

	while (!match_found)
	{
		// Generate random seed
		gen_rand_buffer(seed, 32);

		// Generate keys based on the seed and check for a match;
		// increment the seed instead of generating a new one as an optimisation.
		for (size_t i = 0; i < 512; i++)
		{
			increment_int256(seed);
			compact_ed25519_keygen(private_key, public_key, seed);

			for (size_t j = 0; j < match_spec_count; j++)
			{
				bool match_test = true;
				for (size_t k = 0; k < match_spec_size; k++)
				{
					match_test &= (match_specs[j][k] == public_key[k]);
				}

				if (match_test)
				{
					// Match found, save seed, log to console, break out of all loops and close thread
					memcpy(match_seed, seed, 32);

					std::cout << "\n[MATCH FOUND]" << std::endl;

					std::cout << "public key: [";
					for (size_t l = 0; l < 32; l++)
					{
						if (public_key[l] < 0x10)
							std::cout << '0';

						std::cout << std::hex << (int)public_key[l] << std::dec;

						if (l != 31)
							std::cout << ' ';
					}
					std::cout << "]" << std::endl;

					std::cout << "private key: [";
					for (size_t l = 0; l < 64; l++)
					{
						if (private_key[l] < 0x10)
							std::cout << '0';

						std::cout << std::hex << (int)private_key[l] << std::dec;

						if (l != 63)
							std::cout << ' ';
					}
					std::cout << "]" << std::endl;

					std::cout << "random seed: [";
					for (size_t l = 0; l < 32; l++)
					{
						if (seed[l] < 0x10)
							std::cout << '0';

						std::cout << std::hex << (int)seed[l] << std::dec;

						if (l != 31)
							std::cout << ' ';
					}
					std::cout << "]\n" << std::endl;
					
					match_found = true;
					i = 4096;
					j = match_spec_count;

					break;
				}
			}
		}
	}

	// Loop has exited, meaning a match has been found
	std::cout << "[" << std::this_thread::get_id() << "] Thread exited." << std::endl;
	return;
}

int main()
{
	std::string address;

	std::cout << "[TOR ADDRESS GENERATOR]" << std::endl;
	std::cout << "Enter the beginning of the address ('a-z', '0-7', lowercase, 51 characters max): " << std::endl;
	std::cout << ">> ";
	std::cin >> address;

	// Verify address length
	if (address.length() > 51)
	{
		address.erase(51, address.length() - 51);
		std::cout << "[WARNING]: Address longer than expected, Truncated." << std::endl;
	}

	// Check for invalid characters
	const char base32_charset[32] =
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
		'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
		'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
		'y', 'z', '2', '3', '4', '5', '6', '7',
	};

	for (size_t i = 0; i < address.length(); i++)
	{
		bool addr_valid = false;

		for (size_t j = 0; j < 32; j++)
		{
			addr_valid |= (address[i] == base32_charset[j]);
		}

		if (!addr_valid)
		{
			std::cout << "[ERROR]: Unexpected character in address. Exiting..." << std::endl;
			std::cout << "Press any key to exit..." << std::endl;

			std::cin.get();
			std::cin.get();
			return -1;
		}
	}
	// Decode address into byte interpretations
	base32_decode(address, match_specs, match_spec_size, match_spec_count);

	// Determine thread count to use
	size_t thread_count = std::thread::hardware_concurrency();
	if (thread_count == 0)
		thread_count = 4;

	std::cout << "Enter number of threads to use (for best performance, use "
		<< thread_count << "): " << std::endl;
	std::cout << ">> ";
	std::cin >> thread_count;

	if (thread_count == 0)
	{
		std::cout << "[ERROR] Invalid thread count specified (0). Exiting..." << std::endl;
		std::cout << "Press any key to exit..." << std::endl;

		std::cin.get();
		std::cin.get();
		return -1;
	}

	// Start threads
	std::cout << "Starting threads, you can always stop the process by pressing Ctrl+C" << std::endl;
	std::vector<std::thread*> worker_threads;

	for (size_t i = 0; i < thread_count; i++)
	{
		std::thread* p_thread = new std::thread(thread_main);
		worker_threads.push_back(p_thread);
	}

	// Join all threads; will block execution until a match is found
	for (size_t i = 0; i < thread_count; i++)
	{
		worker_threads[i]->join();
	}

	std::cout << "All threads have exited successfully." << std::endl;

	// Delete all threads
	for (size_t i = 0; i < thread_count; i++)
	{
		delete worker_threads[i];
	}
	worker_threads.clear();
	worker_threads.shrink_to_fit();

	std::cin.get();
	std::cin.get();

	return 0;
}