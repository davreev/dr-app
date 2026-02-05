#include <dr/app/draw_command.hpp>

#include <algorithm>

namespace dr
{
namespace
{

void apply_uniforms(UniformBlock const block, Span<u8 const> const data)
{
    sg_apply_uniforms(int(block), {data.data(), usize(data.size())});
}

} // namespace

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

void submit_draw_cmds(
    Span<DrawCommand const> const& draw_cmds,
    SlicedArray<u8> const& uniform_data,
    GfxBindings const& pass_bindings)
{
    GfxPipeline::Handle pipeline{};
    void const* geometry = nullptr;
    void const* material = nullptr;
    GfxBindings bindings{};

    // Pass uniforms are assumed to be the first slice
    assert(uniform_data.num_slices() > 0);
    Span<u8 const> const pass_uniform_data = uniform_data[0];

    for (auto const& cmd : draw_cmds)
    {
        if (cmd.pipeline.id != pipeline.id)
        {
            pipeline = cmd.pipeline;
            sg_apply_pipeline(pipeline);

            if (pass_uniform_data.size() > 0)
                apply_uniforms(UniformBlock::Pass, pass_uniform_data);

            geometry = material = nullptr;
            bindings = pass_bindings;
        }

        bool bindings_dirty = false;

        if (cmd.material != material)
        {
            if (cmd.material_uniform_data.size() > 0)
                apply_uniforms(UniformBlock::Material, cmd.material_uniform_data);

            material = cmd.material;
            bindings_dirty = true;
        }

        if (cmd.geometry != geometry)
        {
            if (cmd.geometry_uniform_data.size() > 0)
                apply_uniforms(UniformBlock::Geometry, cmd.geometry_uniform_data);

            geometry = cmd.geometry;
            bindings_dirty = true;
        }

        if (bindings_dirty)
        {
            cmd.set_bindings(cmd, bindings);
            sg_apply_bindings(bindings);
        }

        // Slice index of 0 is treated as invalid for object uniforms
        if (cmd.uniform_slice != 0)
        {
            Span<u8 const> const object_uniform_data = uniform_data[cmd.uniform_slice];
            if (object_uniform_data.size() > 0)
                apply_uniforms(UniformBlock::Object, object_uniform_data);
        }

        sg_draw(cmd.base_element, cmd.num_elements, cmd.num_instances);
    }
}

} // namespace dr