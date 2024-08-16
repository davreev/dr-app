#pragma once

#include <sokol_gfx.h>
#include <sokol_gl.h>

#include <dr/bitwise.hpp>
#include <dr/math_types.hpp>

namespace dr
{

inline void debug_draw_axes(Mat4<f32> const& local_to_view, f32 const scale = 1.0f)
{
    sgl_matrix_mode_modelview();
    sgl_load_matrix(local_to_view.data());

    sgl_begin_lines();
    {
        sgl_c3f(1.0f, 0.3f, 0.3f);
        sgl_v3f(0.0f, 0.0f, 0.0f);
        sgl_v3f(scale, 0.0f, 0.0f);

        sgl_c3f(0.3f, 1.0f, 0.3f);
        sgl_v3f(0.0f, 0.0f, 0.0f);
        sgl_v3f(0.0f, scale, 0.0f);

        sgl_c3f(0.3f, 0.3f, 1.0f);
        sgl_v3f(0.0f, 0.0f, 0.0f);
        sgl_v3f(0.0f, 0.0f, scale);
    }
    sgl_end();
}

void debug_draw_unit_cube_edges(Mat4<f32> const& local_to_view)
{
    sgl_matrix_mode_modelview();
    sgl_load_matrix(local_to_view.data());

    // clang-format off
    static constexpr u8 edge_verts[]{
        0, 1,
        2, 3,
        4, 5,
        6, 7,
        0, 2,
        1, 3,
        4, 6,
        5, 7,
        0, 4,
        1, 5,
        2, 6,
        3, 7,
    };
    // clang-format on

    sgl_begin_lines();
    {
        sgl_c3f(1.0, 1.0, 1.0);
        f32 p[3];

        for (auto const& v : edge_verts)
        {
            unit_cube_corner(v, p);
            sgl_v3f(p[0], p[1], p[2]);
        }
    }
    sgl_end();
}

} // namespace dr