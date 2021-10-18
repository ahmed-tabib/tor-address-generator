#pragma once
#include <string>

void base32_decode(std::string in, char** &out, size_t &out_size, size_t &out_count);