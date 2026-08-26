#include <dr/app/draw_context.hpp>

#include <algorithm>

namespace dr
{
namespace
{

void order_draw_cmds(Span<DrawCommand> const& draw_cmds)
{
    // Draw commands are ordered by pipeline, then material, then geometry
    std::sort(begin(draw_cmds), end(draw_cmds), [](DrawCommand const& a, DrawCommand const& b) {
        if (a.pipeline.id != b.pipeline.id)
            return a.pipeline.id < b.pipeline.id;
        else if (a.material != b.material)
            return a.material < b.material;
        else
            return a.geometry < b.geometry;
    });
}

void apply_uniforms(UniformBlock const block, Span<u8 const> const data)
{
    sg_apply_uniforms(int(block), {data.data(), usize(data.size())});
}

bool has_buffer_offsets(DrawCommand const& cmd)
{
    for (u8 i = 0; i < cmd.num_vertex_slots; ++i)
    {
        if (cmd.buffer_offsets.vertex[i] > 0)
            return true;
    }

    return cmd.buffer_offsets.index > 0;
}

void set_buffer_offsets(DrawCommand const& cmd, sg_bindings& bindings)
{
    for (u8 i = 0; i < cmd.num_vertex_slots; ++i)
        bindings.vertex_buffer_offsets[i] = cmd.buffer_offsets.vertex[i];

    bindings.index_buffer_offset = cmd.buffer_offsets.index;
}

} // namespace

void DrawContext::begin_frame()
{
    draw_cmds.clear();
    vertex_stream.reset();
    index_stream.reset();
    uniform_stream.reset();
}

void DrawContext::submit(PassInfo const& pass)
{
    // NOTE(dr): If either transfer fails, the pass is skipped for a frame since any draw command
    // making use of streams refers to data that wasn't uploaded. We recover on the next frame after
    // stream device buffers have been resized.
    bool transfer_ok = true;
    transfer_ok &= vertex_stream.transfer();
    transfer_ok &= index_stream.transfer();
    if (!transfer_ok)
    {
        draw_cmds.clear();
        return;
    }

    order_draw_cmds(as_span(draw_cmds));

    GfxPipeline::Handle pipeline{};
    void const* prev_geometry = nullptr;
    void const* prev_material = nullptr;

    for (auto const& cmd : draw_cmds)
    {
        if (cmd.pipeline.id != pipeline.id)
        {
            pipeline = cmd.pipeline;
            sg_apply_pipeline(pipeline);

            if (pass.uniform_data.size() > 0)
                apply_uniforms(UniformBlock::Pass, pass.uniform_data);

            prev_geometry = prev_material = nullptr;
        }

        bool bindings_dirty = false;

        if (cmd.material != prev_material)
        {
            if (cmd.uniform_slices.material != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_stream[cmd.uniform_slices.material];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Material, data);
            }

            prev_material = cmd.material;
            bindings_dirty = true;
        }

        if (cmd.geometry != prev_geometry)
        {
            if (cmd.uniform_slices.geometry != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_stream[cmd.uniform_slices.geometry];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Geometry, data);
            }

            prev_geometry = cmd.geometry;
            bindings_dirty = true;
        }

        // NOTE(dr): Force rebind if the current command uses buffer offsets. Clearing prev_geometry
        // ensures we rebind on the next command even if it has the same source geometry.
        if (has_buffer_offsets(cmd))
        {
            bindings_dirty = true;
            prev_geometry = {};
        }

        if (bindings_dirty)
        {
            auto bindings = pass.bindings;
            set_buffer_offsets(cmd, bindings);
            cmd.set_bindings(cmd, bindings);
            sg_apply_bindings(bindings);
        }

        if (cmd.uniform_slices.object != invalid_index<i32>)
        {
            Span<u8 const> const data = uniform_stream[cmd.uniform_slices.object];
            if (data.size() > 0)
                apply_uniforms(UniformBlock::Object, data);
        }

        auto const& args = cmd.args;
        sg_draw(args.first_element, args.num_elements, args.num_instances);
    }
}

} // namespace dr