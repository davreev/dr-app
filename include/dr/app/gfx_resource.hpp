#pragma once

#include <sokol_gfx.h>

namespace dr
{

template <typename Handle_, typename Desc_>
struct GfxResource
{
    using Handle = Handle_;
    using Desc = Desc_;

    /// Allocates a new resource without initializing it
    static GfxResource alloc();

    /// Allocates and initializes a new resource
    static GfxResource make(Desc const& desc);

    GfxResource() = default;

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

    /// Implicit conversion to the resource's unique handle
    operator Handle() const { return handle_; }

    /// Returns the resource's unique handle
    Handle handle() const { return handle_; }

    /// Initializes the resource. If the resource has already been initialized, this will
    /// reinitialize it.
    void init(Desc const& desc);

    /// True if the resource has a valid handle
    bool is_valid() const { return handle_.id != SG_INVALID_ID; }

    /// True if the resource has been initialized
    bool is_init() const { return query_state() == SG_RESOURCESTATE_VALID; }

  private:
    Handle handle_{};

    GfxResource(Handle const handle) : handle_{handle} {}

    sg_resource_state query_state() const;

    void destroy();
};

using GfxAttachments = GfxResource<sg_attachments, sg_attachments_desc>;
using GfxPipeline = GfxResource<sg_pipeline, sg_pipeline_desc>;
using GfxShader = GfxResource<sg_shader, sg_shader_desc>;
using GfxBuffer = GfxResource<sg_buffer, sg_buffer_desc>;
using GfxImage = GfxResource<sg_image, sg_image_desc>;
using GfxSampler = GfxResource<sg_sampler, sg_sampler_desc>;

} // namespace dr