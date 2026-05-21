#pragma once

#include <dr/basic_types.hpp>
#include <dr/hash_map.hpp>
#include <dr/sliced_array.hpp>
#include <dr/span.hpp>

#include <dr/app/draw_command.hpp>
#include <dr/app/draw_streams.hpp>
#include <dr/app/gfx_resource.hpp>

namespace dr
{

struct DrawContext
{
    DynamicArray<DrawCommand> draw_cmds;
    VertexStream vertex_stream;
    IndexStream<i32> index_stream;
    UniformStream uniform_stream;

    struct PassInfo
    {
        sg_bindings bindings;
        Span<u8 const> uniform_data;
    };

    void submit_draw_cmds(PassInfo const& pass = {});
};

} // namespace dr