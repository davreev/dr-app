#pragma once

#include <dr/basic_types.hpp>
#include <dr/sliced_array.hpp>
#include <dr/span.hpp>

#include <dr/app/gfx_resource.hpp>

namespace dr
{

struct DrawCommand
{
    GfxPipeline::Handle pipeline{};
    void const* material{};
    void const* geometry{};
    void (*set_bindings)(DrawCommand const& self, GfxBindings& bindings);
    Span<u8 const> material_uniform_data;
    Span<u8 const> geometry_uniform_data;
    i32 uniform_slice{};
    i32 base_element{};
    i32 num_elements{};
    i32 num_instances{};
};

enum struct UniformBlock : u8
{
    Pass = 0,
    Material,
    Geometry,
    Object,
};

// Orders draw commands to minimize state changes during submission
void order_draw_cmds(Span<DrawCommand> const& draw_cmds);

void submit_draw_cmds(
    Span<DrawCommand const> const& draw_cmds,
    SlicedArray<u8> const& uniform_data,
    GfxBindings const& pass_bindings = {});

} // namespace dr