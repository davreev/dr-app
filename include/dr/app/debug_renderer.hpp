#pragma once

#include <dr/basic_types.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/function_ref.hpp>
#include <dr/math_types.hpp>
#include <dr/span.hpp>
#include <dr/transform.hpp>

#include <dr/app/draw_streams.hpp>

namespace dr
{

/// Immediate-mode renderer for debug geometry
struct DebugRenderer
{
    /// Determines how a primitive is composited. Passes are drawn in the order listed.
    enum struct Pass : u8
    {
        /// Depth tested and written, coverage resolves via alpha to coverage
        Opaque = 0,
        /// Depth tested but not written, blended, and depth sorted within each primitive type
        Transparent,
        /// No depth test or write, blended, resolved purely by draw order
        Overlay,
        _Count,
    };

    enum struct LineCap : u8
    {
        Butt = 0,
        Square,
        Round,
        _Count,
    };

    enum struct PointShape : u8
    {
        Disc = 0,
        Square,
        Ring,
        Cross,
        _Count,
    };

    struct LineStyle
    {
        Vec4<f32> color = {1.0, 1.0, 1.0, 1.0};
        f32 width = 2.0;
        LineCap cap = LineCap::Round;
        Pass pass = Pass::Opaque;
    };

    struct PointStyle
    {
        Vec4<f32> color = {1.0, 1.0, 1.0, 1.0};
        f32 size = 4.0;
        PointShape shape = PointShape::Disc;
        Pass pass = Pass::Opaque;
    };

    struct View
    {
        Mat4<f32> world_to_clip = Mat4<f32>::Identity();
        struct
        {
            Vec2<f32> size;
            f32 dpi_scale = 1.0;
        } viewport;
    };

    struct LineInstance
    {
        f32 start[3] = {};
        f32 end[3] = {};
        u8 color[4] = {};
        f32 width = {};
        u8 flags[4] = {};
    };

    struct PointInstance
    {
        f32 position[3] = {};
        u8 color[4] = {};
        f32 size = {};
        u8 flags[4] = {};
    };

    /// Call once per frame before drawing any geometry
    void begin_frame();

    /// Clears accumulated geometry. Can be called mutliple times within a single frame.
    void clear();

    /// Submits draw calls for accumulated geometry. Must be called within an active render pass.
    void submit(View const& view);

    /*
        Draw commands
    */

    void draw_line(Vec3<f32> const& a, Vec3<f32> const& b) { draw_line(a, b, {}); }
    void draw_line(Vec3<f32> const& a, Vec3<f32> const& b, LineStyle const& style);

    void draw_point(Vec3<f32> const& p) { draw_point(p, {}); }
    void draw_point(Vec3<f32> const& p, PointStyle const& style);

    struct Polyline
    {
        Span<Vec3<f32> const> points;
        bool closed = false;
    };

    void draw_polyline(Polyline const& poly) { draw_polyline(poly, {}); }
    void draw_polyline(Polyline const& poly, LineStyle const& style);

    struct Curve
    {
        Affine3<f32> transform;
        FunctionRef<Vec3<f32>(f32)> eval;
        Vec2<f32> domain = {0.0, 1.0};
        Vec2<f32> segment_trim = {0.0, 1.0};
        i32 num_segments = 32;

        Vec3<f32> operator()(f32 t) const;
    };

    void draw_curve(Curve const& curve) { draw_curve(curve, {}); }
    void draw_curve(Curve const& curve, LineStyle const& style);

    void draw_curve_points(Curve const& curve) { draw_curve_points(curve, {}); }
    void draw_curve_points(Curve const& curve, PointStyle const& style);

    struct Box
    {
        Affine3<f32> transform;
        Vec3<f32> from = {0.0, 0.0, 0.0};
        Vec3<f32> to = {1.0, 1.0, 1.0};
    };

    void draw_box(Box const& box) { draw_box(box, {}); }
    void draw_box(Box const& box, LineStyle const& style);

    struct Circle
    {
        Affine3<f32> transform;
        Vec2<f32> center;
        f32 radius = 1.0;
        i32 num_segments = 32;
    };

    void draw_circle(Circle const& circ) { draw_circle(circ, {}); }
    void draw_circle(Circle const& circ, LineStyle const& style);

    struct Dart
    {
        Vec3<f32> start;
        Vec3<f32> end;
        Vec3<f32> up;
        f32 head_scale = 0.15;
    };

    void draw_dart(Dart const& dart) { draw_dart(dart, {}); }
    void draw_dart(Dart const& dart, LineStyle const& style);

    struct Frame
    {
        Affine3<f32> transform;
        f32 scale = 1.0;
    };

    struct FrameStyle
    {
        Vec4<f32> color_x = {1.0, 0.3, 0.3, 1.0};
        Vec4<f32> color_y = {0.3, 1.0, 0.3, 1.0};
        Vec4<f32> color_z = {0.3, 0.3, 1.0, 1.0};
        f32 width = 2.0;
        struct
        {
            Vec4<f32> color{1.0, 1.0, 1.0, 1.0};
            f32 rel_size = 1.5;
        } origin;
    };

    void draw_frame(Frame const& frame) { draw_frame(frame, {}); }
    void draw_frame(Frame const& frame, FrameStyle const& style);

    struct Grid
    {
        Affine3<f32> transform;
        Vec2<f32> spacing = {1.0, 1.0};
        Vec2<i32> num_cells = {16, 16};
        bool centered = false;
    };

    void draw_grid_lines(Grid const& grid) { draw_grid_lines(grid, {}); }
    void draw_grid_lines(Grid const& grid, LineStyle const& style);

    void draw_grid_points(Grid const& grid) { draw_grid_points(grid, {}); }
    void draw_grid_points(Grid const& grid, PointStyle const& style);

  private:
    DynamicArray<LineInstance> lines_[u8(Pass::_Count)];
    DynamicArray<PointInstance> points_[u8(Pass::_Count)];
    VertexStream stream_;
};

} // namespace dr
