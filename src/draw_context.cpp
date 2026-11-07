#include <dr/app/draw_context.hpp>

#include <algorithm>
#include <cassert>

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

bool binding_offsets_differ(DrawCommand const& a, DrawCommand const& b)
{
    for (u8 i = 0; i < DrawCommand::num_vertex_slots; ++i)
    {
        if (a.bindings.vertex_offsets[i] != b.bindings.vertex_offsets[i])
            return true;
    }

    return a.bindings.index_offset != b.bindings.index_offset;
}

void set_binding_offsets(DrawCommand const& cmd, sg_bindings& bindings)
{
    for (u8 i = 0; i < cmd.num_vertex_slots; ++i)
        bindings.vertex_buffer_offsets[i] = cmd.bindings.vertex_offsets[i];

    bindings.index_buffer_offset = cmd.bindings.index_offset;
}

} // namespace

void DrawContext::next_frame()
{
    assert(stage_ != Stage::Record);

    draw_cmds_.clear();
    batches_.clear();

    vertex_stream.reset();
    index_stream.reset();
    uniform_stream.reset();

    stage_ = Stage::Record;
}

void DrawContext::begin_batch(BatchInfo const& info)
{
    assert(stage_ == Stage::Record);
    assert(!curr_batch());

    batches_.push_back({
        .bindings = info.bindings,
        .uniform_slice = uniform_stream.push(info.uniform_data),
        .cmd_from = i32(size(draw_cmds_)),
    });
}

void DrawContext::draw(DrawCommand const& cmd)
{
    assert(stage_ == Stage::Record);
    assert(curr_batch());
    draw_cmds_.push_back(cmd);
}

DrawContext::BatchHandle DrawContext::end_batch()
{
    assert(stage_ == Stage::Record);

    Batch* batch = curr_batch();
    assert(batch);

    batch->cmd_to = size(draw_cmds_);
    assert(batch->cmd_to >= batch->cmd_from);

    return {i32(size(batches_) - 1)};
}

DrawContext::BatchHandle DrawContext::repeat_batch(BatchHandle handle, BatchInfo const& info)
{
    assert(stage_ == Stage::Record);
    assert(!curr_batch());

    Batch const& other = batches_[handle.index];
    batches_.push_back({
        .bindings = info.bindings,
        .uniform_slice = uniform_stream.push(info.uniform_data),
        .cmd_from = other.cmd_from,
        .cmd_to = other.cmd_to,
        .is_repeat = true,
    });

    return {i32(size(batches_) - 1)};
}

DrawContext::Batch* DrawContext::curr_batch()
{
    if (batches_.empty())
        return nullptr;

    Batch& b = batches_.back();
    return (b.cmd_to == invalid_index<i32>) ? &b : nullptr;
}

void DrawContext::commit_batches()
{
    assert(stage_ == Stage::Record);

    vertex_stream.transfer();
    index_stream.transfer();

    // Sort each batch of draw cmds
    auto cmds = as_span(draw_cmds_);
    for (Batch const& b : batches_)
    {
        // Don't re-sort a repeated batch
        if (b.is_repeat)
            continue;

        order_draw_cmds(cmds.range(b.cmd_from, b.cmd_to));
    }

    stage_ = Stage::Submit;
}

void DrawContext::submit_batch(BatchHandle const handle)
{
    assert(stage_ == Stage::Submit);

    Batch const& batch = batches_[handle.index];

    DrawCommand default_cmd{};
    DrawCommand const* prev_cmd = &default_cmd;

    for (auto const& cmd : as_span(draw_cmds_).range(batch.cmd_from, batch.cmd_to))
    {
        bool pipeline_changed = false;

        if (cmd.pipeline.id != prev_cmd->pipeline.id)
        {
            sg_apply_pipeline(cmd.pipeline);

            if (batch.uniform_slice != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_stream[batch.uniform_slice];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Pass, data);
            }

            pipeline_changed = true;
        }

        bool bindings_dirty = false;

        if (pipeline_changed || cmd.material != prev_cmd->material)
        {
            if (cmd.uniform_slices.material != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_stream[cmd.uniform_slices.material];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Material, data);
            }

            bindings_dirty = true;
        }

        if (pipeline_changed || cmd.geometry != prev_cmd->geometry)
        {
            if (cmd.uniform_slices.geometry != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_stream[cmd.uniform_slices.geometry];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Geometry, data);
            }

            bindings_dirty = true;
        }

        // Rebind if pipeline, resources, or binding offsets have changed
        if (bindings_dirty || binding_offsets_differ(cmd, *prev_cmd))
        {
            auto bindings = batch.bindings;
            set_binding_offsets(cmd, bindings);
            cmd.bindings.assign(cmd, bindings);
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

        prev_cmd = &cmd;
    }
}

} // namespace dr
