#pragma once

#include <cmath>

#include <fstream>
#include <vector>

#include <sokol_app.h>

#include <dr/math_types.hpp>
#include <dr/span.hpp>

#include "app.hpp"
#include "camera.hpp"

namespace dr
{

/// Reads a file to a given buffer
bool read_text_file(char const* path, std::string& buffer);

/// Reads a file to a given buffer
bool read_binary_file(char const* path, std::vector<u8>& buffer);

} // namespace dr