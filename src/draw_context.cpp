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

void submit_draw_cmds(
    DrawContext::PassInfo const& pass,
    Span<DrawCommand const> const& draw_cmds,
    SlicedArray<u8> const& uniform_data)
{
    GfxPipeline::Handle pipeline{};
    void const* geometry = nullptr;
    void const* material = nullptr;

    for (auto const& cmd : draw_cmds)
    {
        if (cmd.pipeline.id != pipeline.id)
        {
            pipeline = cmd.pipeline;
            sg_apply_pipeline(pipeline);

            if (pass.uniform_data.size() > 0)
                apply_uniforms(UniformBlock::Pass, pass.uniform_data);

            geometry = material = nullptr;
        }

        bool bindings_dirty = false;

        if (cmd.material != material)
        {
            if (cmd.uniform_slices.material != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_data[cmd.uniform_slices.material];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Material, data);
            }

            material = cmd.material;
            bindings_dirty = true;
        }

        if (cmd.geometry != geometry)
        {
            if (cmd.uniform_slices.geometry != invalid_index<i32>)
            {
                Span<u8 const> const data = uniform_data[cmd.uniform_slices.geometry];
                if (data.size() > 0)
                    apply_uniforms(UniformBlock::Geometry, data);
            }

            geometry = cmd.geometry;
            bindings_dirty = true;
        }

        if (bindings_dirty)
        {
            auto bindings = pass.bindings;
            cmd.set_bindings(cmd, bindings);
            sg_apply_bindings(bindings);
        }

        if (cmd.uniform_slices.object != invalid_index<i32>)
        {
            Span<u8 const> const data = uniform_data[cmd.uniform_slices.object];
            if (data.size() > 0)
                apply_uniforms(UniformBlock::Object, data);
        }

        sg_draw(cmd.base_element, cmd.num_elements, cmd.num_instances);
    }
}

} // namespace

i32 DrawContext::push_uniforms(Span<u8 const> const& data)
{
    i32 const slice = uniform_data_.num_slices();
    uniform_data_.push_back(data);
    return slice;
}

i32 DrawContext::push_uniforms_once(void const* key, Span<u8 const> const& data)
{
    auto const [it, ok] = slices_.try_emplace(key);
    if (ok)
        it->second = push_uniforms(data);

    return it->second;
}

void DrawContext::submit_draw_cmds(PassInfo const& pass)
{
    geometry.update_device_buffers();

    order_draw_cmds(as_span(draw_cmds));
    dr::submit_draw_cmds(pass, as_span(draw_cmds), uniform_data_);

    draw_cmds.clear();
    geometry.clear();
    uniform_data_.clear();
    slices_.clear();
}

} // namespace dr