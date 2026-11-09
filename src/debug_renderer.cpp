#include <dr/app/debug_renderer.hpp>

#include <cassert>
#include <cmath>

#include <sokol_gfx.h>

#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/math_constants.hpp>
#include <dr/memory.hpp>

#include "debug_renderer_shader_src.hpp"

namespace dr
{
namespace
{

using LineStyle = DebugRenderer::LineStyle;
using PointStyle = DebugRenderer::PointStyle;
using View = DebugRenderer::View;
using LineInstance = DebugRenderer::LineInstance;
using PointInstance = DebugRenderer::PointInstance;

constexpr u8 num_passes = u8(DebugRenderer::Pass::_Count);

// NOTE(dr): The assigned shader stage doesn't appear to matter when using OpenGL backends
constexpr sg_shader_stage shader_stage_any = SG_SHADERSTAGE_VERTEX;

struct PassParams
{
    f32 world_to_clip[16]{};
    f32 viewport[3]{};

    static PassParams make(View const& view)
    {
        PassParams p;
        as_mat<4, 4>(p.world_to_clip) = view.world_to_clip;
        as_vec<2>(p.viewport) = view.viewport.size;
        p.viewport[2] = view.viewport.dpi_scale;
        return p;
    }

    static sg_shader_uniform_block uniform_block()
    {
        return {
            .stage = shader_stage_any,
            .size = sizeof(PassParams),
            .glsl_uniforms{
                {
                    .type = SG_UNIFORMTYPE_MAT4,
                    .glsl_name = "u_world_to_clip",
                },
                {
                    .type = SG_UNIFORMTYPE_FLOAT3,
                    .glsl_name = "u_viewport",
                },
            },
        };
    }
};

sg_pipeline_desc opaque_pipeline_desc(GfxShader::Handle const shader)
{
    return {
        .shader = shader,
        .depth{
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
        .cull_mode = SG_CULLMODE_NONE,
        .alpha_to_coverage_enabled = true,
    };
}

sg_pipeline_desc overlay_pipeline_desc(GfxShader::Handle const shader)
{
    return {
        .shader = shader,
        .depth{
            .compare = SG_COMPAREFUNC_ALWAYS,
            .write_enabled = false,
        },
        .colors{
            {
                .blend{
                    .enabled = true,
                    .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                    .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                    .src_factor_alpha = SG_BLENDFACTOR_ONE,
                    .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                },
            },
        },
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
        .cull_mode = SG_CULLMODE_NONE,
    };
}

template <typename Instance>
struct Impl;

template <>
struct Impl<LineInstance>
{
    inline static GfxShader shader;
    inline static GfxPipeline pipelines[num_passes];

    static void init_pipelines()
    {
        shader.init({
            .vertex_func{.source = line_vertex_src},
            .fragment_func{.source = line_fragment_src},
            .uniform_blocks{PassParams::uniform_block()},
        });
        assert(shader.is_valid());

        sg_pipeline_desc descs[]{
            opaque_pipeline_desc(shader),
            overlay_pipeline_desc(shader),
        };
        static_assert(size(descs) == num_passes);

        for (u8 i = 0; i < num_passes; ++i)
        {
            descs[i].layout = {
                .buffers{{.step_func = SG_VERTEXSTEP_PER_INSTANCE}},
                .attrs{
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3}, // start
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3}, // end
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT2}, // trim
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT}, // width
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_UBYTE4N}, // color
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_UBYTE4}, // flags
                },
            };
            pipelines[i] = GfxPipeline::make(descs[i]);
            assert(pipelines[i].is_valid());
        }
    }
};

template <>
struct Impl<PointInstance>
{
    inline static GfxShader shader;
    inline static GfxPipeline pipelines[num_passes];

    static void init_pipelines()
    {
        shader.init({
            .vertex_func{.source = point_vertex_src},
            .fragment_func{.source = point_fragment_src},
            .uniform_blocks{PassParams::uniform_block()},
        });
        assert(shader.is_valid());

        sg_pipeline_desc descs[]{
            opaque_pipeline_desc(shader),
            overlay_pipeline_desc(shader),
        };
        static_assert(size(descs) == num_passes);

        for (u8 i = 0; i < num_passes; ++i)
        {
            descs[i].layout = {
                .buffers{{.step_func = SG_VERTEXSTEP_PER_INSTANCE}},
                .attrs{
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT3}, // position
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_FLOAT}, // size
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_UBYTE4N}, // color
                    {.buffer_index = 0, .format = SG_VERTEXFORMAT_UBYTE4}, // flags
                },
            };
            pipelines[i] = GfxPipeline::make(descs[i]);
            assert(pipelines[i].is_valid());
        }
    }
};

void init_resources_once()
{
    static bool needs_init = true;
    if (needs_init)
    {
        Impl<LineInstance>::init_pipelines();
        Impl<PointInstance>::init_pipelines();
        needs_init = false;
    }
}

void pack_color(Vec4<f32> const& src, u8 (&dst)[4])
{
    for (isize i = 0; i < 4; ++i)
        dst[i] = u8(clamp(256.0f * src[i], 0.0f, 255.0f));
}

LineInstance make_line(Vec3<f32> const& start, Vec3<f32> const& end, LineStyle const& style)
{
    LineInstance result{};
    as_vec(result.start) = start;
    as_vec(result.end) = end;
    as_vec(result.trim) = style.trim;
    result.width = style.width;
    pack_color(style.color, result.color);
    result.flags[0] = u8(style.cap);
    return result;
}

PointInstance make_point(Vec3<f32> const& position, PointStyle const& style)
{
    PointInstance result{};
    as_vec(result.position) = position;
    result.size = style.size;
    pack_color(style.color, result.color);
    result.flags[0] = u8(style.shape);
    return result;
}

} // namespace

void DebugRenderer::next_frame()
{
    assert(stage_ != Stage::Record);

    init_resources_once();

    draw_calls_.clear();
    batches_.clear();
    stream_.reset();

    stage_ = Stage::Record;
}

void DebugRenderer::begin_batch(View const& view)
{
    assert(stage_ == Stage::Record);
    assert(!curr_batch());

    for (u8 i = 0; i < num_passes; ++i)
    {
        lines_[i].clear();
        points_[i].clear();
    }

    batches_.push_back({
        .view = view,
        .draw_from = i32(size(draw_calls_)),
    });
}

DebugRenderer::BatchHandle DebugRenderer::end_batch()
{
    assert(stage_ == Stage::Record);

    Batch* batch = curr_batch();
    assert(batch);

    // Stage instance data for each non-empty bucket
    auto stage = [&](GfxPipeline::Handle const pipeline,
                     Span<u8 const> const& instance_data,
                     i32 const num_instances) {
        if (num_instances == 0)
            return;

        draw_calls_.push_back({
            .pipeline = pipeline,
            .buffer_offset = stream_.push(instance_data),
            .num_instances = num_instances,
        });
    };

    for (u8 i = 0; i < num_passes; ++i)
    {
        auto const& lines = lines_[i];
        stage(Impl<LineInstance>::pipelines[i], as<u8>(as_span(lines)), i32(lines.size()));

        auto const& points = points_[i];
        stage(Impl<PointInstance>::pipelines[i], as<u8>(as_span(points)), i32(points.size()));
    }

    batch->draw_to = i32(size(draw_calls_));
    return {i32(size(batches_) - 1)};
}

DebugRenderer::BatchHandle DebugRenderer::repeat_batch(BatchHandle const handle, View const& view)
{
    assert(stage_ == Stage::Record);
    assert(!curr_batch());

    Batch const& other = batches_[handle.index];
    batches_.push_back({
        .view = view,
        .draw_from = other.draw_from,
        .draw_to = other.draw_to,
    });

    return {i32(size(batches_) - 1)};
}

DebugRenderer::Batch* DebugRenderer::curr_batch()
{
    if (batches_.empty())
        return nullptr;

    Batch& b = batches_.back();
    return (b.draw_to == invalid_index<i32>) ? &b : nullptr;
}

void DebugRenderer::commit_batches()
{
    assert(stage_ == Stage::Record);
    assert(!curr_batch());

    stream_.transfer();
    stage_ = Stage::Submit;
}

void DebugRenderer::submit_batch(BatchHandle const handle)
{
    assert(stage_ == Stage::Submit);

    Batch const& batch = batches_[handle.index];
    auto const pass_params = PassParams::make(batch.view);

    for (auto const& call : as_span(draw_calls_).range(batch.draw_from, batch.draw_to))
    {
        sg_apply_pipeline(call.pipeline);
        sg_apply_uniforms(0, {&pass_params, sizeof(pass_params)});
        sg_apply_bindings({
            .vertex_buffers{stream_.device_buffer()},
            .vertex_buffer_offsets{call.buffer_offset},
        });
        sg_draw(0, 4, call.num_instances);
    }
}

/*
    Draw commands
*/

void DebugRenderer::draw_line(Vec3<f32> const& a, Vec3<f32> const& b, LineStyle const& style)
{
    assert(stage_ == Stage::Record);
    assert(curr_batch());

    lines_[u8(style.pass)].push_back(make_line(a, b, style));
}

void DebugRenderer::draw_point(Vec3<f32> const& p, PointStyle const& style)
{
    assert(stage_ == Stage::Record);
    assert(curr_batch());

    points_[u8(style.pass)].push_back(make_point(p, style));
}

void DebugRenderer::draw_polyline(Polyline const& poly, LineStyle const& style)
{
    assert(stage_ == Stage::Record);
    assert(curr_batch());

    auto const& pts = poly.points;
    if (pts.size() < 2)
        return;

    auto& dst = lines_[u8(style.pass)];
    dst.reserve(dst.size() + usize(pts.size()));

    isize const last = pts.size() - 1;
    isize i = 0;
    for (; i < last; ++i)
        dst.push_back(make_line(pts[i], pts[i + 1], style));

    if (poly.closed)
        dst.push_back(make_line(pts[i], pts[0], style));
}

Vec3<f32> DebugRenderer::Curve::operator()(f32 const t) const
{
    return transform.apply(eval(lerp(domain[0], domain[1], t)));
}

void DebugRenderer::draw_curve(Curve const& curve, LineStyle const& style)
{
    f32 const dt = 1.0 / curve.num_segments;
    Vec3<f32> a = curve(0.0);

    for (int i = 1; i <= curve.num_segments; ++i)
    {
        Vec3<f32> const b = curve(i * dt);
        Vec3<f32> const ab = b - a;
        draw_line(a + curve.segment_trim[0] * ab, a + curve.segment_trim[1] * ab, style);
        a = b;
    }
}

void DebugRenderer::draw_curve_points(Curve const& curve, PointStyle const& style)
{
    f32 const dt = 1.0 / curve.num_segments;
    for (int i = 0; i <= curve.num_segments; ++i)
        draw_point(curve(i * dt), style);
}

void DebugRenderer::draw_box(Box const& box, LineStyle const& style)
{
    Vec3<f32> corners[8];
    for (u8 i = 0; i < 8; ++i)
    {
        corners[i] = {
            (i & 1) ? box.to[0] : box.from[0],
            (i & 2) ? box.to[1] : box.from[1],
            (i & 4) ? box.to[2] : box.from[2],
        };
    }

    auto const& xform = box.transform;
    for (auto& p : corners)
        p = xform.apply(p);

    static constexpr u8 edges[]{
        0, 1, 2, 3, 4, 5, 6, 7, // Along x
        0, 2, 1, 3, 4, 6, 5, 7, // Along y
        0, 4, 1, 5, 2, 6, 3, 7, // Along z
    };

    for (isize i = 0; i < size(edges); i += 2)
        draw_line(corners[edges[i]], corners[edges[i + 1]], style);
}

void DebugRenderer::draw_circle(Circle const& circ, LineStyle const& style)
{
    constexpr f32 two_pi = 2.0f * pi<f32>;

    // Bake circle center into translation component of transform
    auto xform = circ.transform;
    xform.translation += xform.linear.leftCols<2>() * circ.center;

    auto point_at = [&](f32 const t) -> Vec3<f32> {
        f32 const ang = two_pi * t;
        return {circ.radius * std::cos(ang), circ.radius * std::sin(ang), 0.0};
    };

    f32 const dt = 1.0 / circ.num_segments;
    Vec3<f32> a = xform.apply(point_at(0.0));

    for (int i = 1; i <= circ.num_segments; ++i)
    {
        Vec3<f32> const b = xform.apply(point_at(i * dt));
        draw_line(a, b, style);
        a = b;
    }
}

void DebugRenderer::draw_dart(Dart const& dart, LineStyle const& style)
{
    draw_line(dart.start, dart.end, style);

    Vec3<f32> const x = dart.head_scale * (dart.end - dart.start);
    Vec3<f32> const y = x.cross(dart.up).normalized();
    Vec3<f32> const z = y.cross(x);

    Vec3<f32> const d = 0.5 * z - x;
    draw_line(dart.end, dart.end + d, style);
}

void DebugRenderer::draw_frame(Frame const& frame, FrameStyle const& style)
{
    Vec3<f32> const& origin = frame.transform.translation;
    Mat3<f32> const axes = frame.scale * frame.transform.linear;
    f32 const origin_size = style.origin.rel_size * style.width;
    Vec2<f32> const trim{style.origin.rel_trim * style.width, 0.0};
    draw_line(
        origin,
        origin + axes.col(0),
        {.color = style.color_x, .trim = trim, .width = style.width});
    draw_line(
        origin,
        origin + axes.col(1),
        {.color = style.color_y, .trim = trim, .width = style.width});
    draw_line(
        origin,
        origin + axes.col(2),
        {.color = style.color_z, .trim = trim, .width = style.width});
    draw_point(
        origin,
        {.color = style.origin.color, .size = origin_size, .shape = PointShape::Disc});
}

void DebugRenderer::draw_grid_lines(Grid const& grid, LineStyle const& style)
{
    if (grid.num_cells.minCoeff() < 1)
        return;

    Vec2<f32> const extent = grid.spacing.array() * grid.num_cells.array().cast<f32>();
    Vec2<f32> const offset = grid.centered ? -0.5 * extent : Vec2<f32>{};

    auto const& xform = grid.transform;

    // x lines
    for (i32 i = 0; i <= grid.num_cells.y(); ++i)
    {
        Vec3<f32> const a{offset.x(), offset.y() + i * grid.spacing.y(), 0.0};
        Vec3<f32> const b{a.x() + extent.x(), a.y(), 0.0};
        draw_line(xform.apply(a), xform.apply(b), style);
    }

    // y lines
    for (i32 i = 0; i <= grid.num_cells.x(); ++i)
    {
        Vec3<f32> const a{offset.x() + i * grid.spacing.x(), offset.y(), 0.0};
        Vec3<f32> const b{a.x(), a.y() + extent.y(), 0.0};
        draw_line(xform.apply(a), xform.apply(b), style);
    }
}

void DebugRenderer::draw_grid_points(Grid const& grid, PointStyle const& style)
{
    if (grid.num_cells.minCoeff() < 1)
        return;

    Vec2<f32> const extent = grid.spacing.array() * grid.num_cells.array().cast<f32>();
    Vec2<f32> const offset = grid.centered ? -0.5 * extent : Vec2<f32>{};

    auto const& xform = grid.transform;

    for (i32 i = 0; i <= grid.num_cells.y(); ++i)
    {
        f32 const dy = grid.spacing.y() * i;

        for (i32 j = 0; j <= grid.num_cells.x(); ++j)
        {
            f32 const dx = grid.spacing.x() * j;
            Vec3<f32> const p{offset.x() + dx, offset.y() + dy, 0.0};
            draw_point(xform.apply(p), style);
        }
    }
}

} // namespace dr
