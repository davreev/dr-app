#pragma once

#include <dr/basic_types.hpp>
#include <dr/hash_map.hpp>
#include <dr/sliced_array.hpp>
#include <dr/span.hpp>

#include <dr/app/draw_command.hpp>
#include <dr/app/geometry_stream.hpp>
#include <dr/app/gfx_resource.hpp>

namespace dr
{

struct DrawContext
{
    DynamicArray<DrawCommand> draw_cmds;
    GeometryStream geometry;

    i32 push_uniforms(Span<u8 const> const& data);
    i32 push_uniforms_once(void const* key, Span<u8 const> const& data);

    struct PassInfo
    {
        sg_bindings bindings;
        Span<u8 const> uniform_data;
    };

    void submit_draw_cmds(PassInfo const& pass = {});

  private:
    SlicedArray<u8> uniform_data_;
    HashMap<void const*, i32> slices_;
};

} // namespace dr