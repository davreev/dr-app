#include <dr/app/file_utils.hpp>

#include <sokol_app.h>

#include <dr/math.hpp>

namespace dr
{

bool read_text_file(char const* const path, std::string& buffer)
{
    std::ifstream in{path, std::ios::in};

    if (in)
    {
        using Iter = std::istreambuf_iterator<char>;
        buffer.assign(Iter{in}, {});
        return true;
    }
    else
    {
        return false;
    }
}

bool read_binary_file(char const* const path, std::vector<u8>& buffer)
{
    std::ifstream in{path, std::ios::in | std::ios::binary};

    if (in)
    {
        using Iter = std::istreambuf_iterator<char>;
        buffer.assign(Iter{in}, {});
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace dr