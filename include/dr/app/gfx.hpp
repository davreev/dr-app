#pragma once

#include <sokol_gfx.h>

namespace dr
{

template <typename Handle_, typename Desc_>
struct GfxResource
{
    using Handle = Handle_;
    using Desc = Desc_;

    GfxResource() = default;

    GfxResource(Desc const& desc);

    GfxResource(GfxResource<Handle, Desc>&& other) noexcept :
        handle_{other.handle_}
    {
        other.handle_ = {};
    }

    GfxResource<Handle, Desc>& operator=(GfxResource<Handle, Desc>&& other) noexcept
    {
        if (this != &other)
        {
            destroy();
            handle_ = other.handle_;
            other.handle_ = {};
        }

        return *this;
    }

    ~GfxResource() { destroy(); }

    constexpr operator Handle() const { return handle_; }

    void alloc();

    void init(Desc const& desc);

    void destroy();

    bool is_alloc() const { return handle_.id != SG_INVALID_ID; }

  private:
    Handle handle_;

    sg_resource_state query_state() const;

    bool is_init() const { return query_state() == SG_RESOURCESTATE_VALID; }
};

using GfxPass = GfxResource<sg_pass, sg_pass_desc>;
using GfxPipeline = GfxResource<sg_pipeline, sg_pipeline_desc>;
using GfxShader = GfxResource<sg_shader, sg_shader_desc>;
using GfxBuffer = GfxResource<sg_buffer, sg_buffer_desc>;
using GfxImage = GfxResource<sg_image, sg_image_desc>;

} // namespace dr