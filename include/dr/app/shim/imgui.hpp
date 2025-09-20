#pragma once

#include <dr/math_types.hpp>

// Inlined as part of ImVec2 class declaration
#define IM_VEC2_CLASS_EXTRA                                       \
    ImVec2(dr::Vec2<float> const& f) { x = f.x(); y = f.y(); }  \
    operator dr::Vec2<float>() const { return {x, y}; }

// Inlined as part of ImVec4 class declaration
#define IM_VEC4_CLASS_EXTRA                                                             \
    ImVec4(dr::Vec4<float> const& f) { x = f.x(); y = f.y(); z = f.z(); w = f.w(); }  \
    operator dr::Vec4<float>() const { return {x, y, z, w}; }

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

}
