#pragma once

#include <imgui.h>

namespace dr
{

struct ImGuiStyles
{
    static void set_default(ImGuiStyle& style);
    // ...
    // ...
};

struct ImGuiColors
{
    static void set_low_contrast_dark(ImVec4* colors);
    static void set_low_contrast_mid(ImVec4* colors);
    static void set_low_contrast_light(ImVec4* colors);
    // ...
    // ...
};

} // namespace dr
