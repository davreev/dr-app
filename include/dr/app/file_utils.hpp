#pragma once

#include <vector>
#include <string>

#include <dr/basic_types.hpp>

namespace dr
{

/// Reads a file to a given buffer
bool read_text_file(char const* path, std::string& buffer);

/// Appends a file to a given buffer
bool append_text_file(char const* path, std::string& buffer);

/// Reads a file to a given buffer
bool read_binary_file(char const* path, std::vector<u8>& buffer);

/// Appends a file to a given buffer
bool append_binary_file(char const* path, std::vector<u8>& buffer);

} // namespace dr