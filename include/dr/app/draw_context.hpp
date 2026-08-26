#pragma once

#include <sokol_gfx.h>

#include <dr/basic_types.hpp>
#include <dr/span.hpp>

#include <dr/app/draw_command.hpp>
#include <dr/app/draw_streams.hpp>

namespace dr
{

struct DrawContext
{
    DynamicArray<DrawCommand> draw_cmds;
    VertexStream vertex_stream;
    IndexStream index_stream;
    UniformStream uniform_stream;

    struct PassInfo
    {
        sg_bindings bindings;
        Span<u8 const> uniform_data;
    };

    /// Call once per frame before adding any draw commands.
    void begin_frame();

    /// Submits accumulated draw commands. Must be called within an active render pass.
    void submit(PassInfo const& pass = {});
};

} // namespace dr