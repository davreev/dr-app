#pragma once

#include <dr/basic_types.hpp>
#include <dr/constants.hpp>

#include <dr/app/gfx_resource.hpp>

namespace dr
{

enum struct UniformBlock : u8
{
    Pass = 0,
    Material,
    Geometry,
    Object,
};

struct DrawCommand
{
    static constexpr u8 num_vertex_slots = SG_MAX_VERTEXBUFFER_BINDSLOTS;

    GfxPipeline::Handle pipeline{};
    void const* material{};
    void const* geometry{};
    void (*set_bindings)(DrawCommand const& self, sg_bindings& bindings);
    struct
    {
        i32 vertex[num_vertex_slots]{};
        i32 index{};
    } buffer_offsets;
    struct
    {
        i32 material{invalid_index<i32>};
        i32 geometry{invalid_index<i32>};
        i32 object{invalid_index<i32>};
    } uniform_slices;
    struct
    {
        i32 first_element{};
        i32 num_elements{};
        i32 num_instances{1};
    } args;
};

} // namespace dr