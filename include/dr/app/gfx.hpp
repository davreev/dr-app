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

    GfxResource(GfxResource<Handle, Desc>&& other) noexcept : handle_{other.handle_}
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

    /// Implicit conversion to the resource's unique handle. This will be invalid if the resource
    /// has not been allocated.
    operator Handle() const { return handle_; }

    /// Returns the resource's unique handle. This will be invalid if the resource has not been
    /// allocated.
    Handle handle() const { return handle_; }

    /// Allocates a unique handle for the resource. This can't be done until the backing graphics
    /// API has been initialized.
    void alloc();

    /// Initializes the resource. If the resource has not yet been allocted, this will allocate
    /// it first. If the resource has already been initialized, this will deinitialize it first.
    void init(Desc const& desc);

    /// Destroys the resource if it has been allocated
    void destroy();

    /// True if the resource has been allocated
    bool is_alloc() const { return handle_.id != SG_INVALID_ID; }

    /// True if the resource has been initialized
    bool is_init() const { return query_state() == SG_RESOURCESTATE_VALID; }

  private:
    using State = sg_resource_state;

    Handle handle_;

    State query_state() const;
};

using GfxPass = GfxResource<sg_pass, sg_pass_desc>;
using GfxPipeline = GfxResource<sg_pipeline, sg_pipeline_desc>;
using GfxShader = GfxResource<sg_shader, sg_shader_desc>;
using GfxBuffer = GfxResource<sg_buffer, sg_buffer_desc>;
using GfxImage = GfxResource<sg_image, sg_image_desc>;

} // namespace dr