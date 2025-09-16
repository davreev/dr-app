#pragma once

#include <dr/math_types.hpp>

namespace dr
{

void debug_draw_axes(Mat4<f32> const& local_to_view, f32 scale = 1.0f, f32 alpha = 1.0f);

void debug_draw_unit_cube_edges(
    Mat4<f32> const& local_to_view,
    Vec4<f32> const& color = {1.0f, 1.0f, 1.0f, 1.0f});

} // namespace dr