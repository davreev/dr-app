#pragma once

#include <sokol_gfx.h>

#include <dr/traits.hpp>

namespace dr
{

template <typename Handle_>
struct GfxResource
{
    using Handle = Handle_;
    using Desc = typename Traits<Handle>::Desc;

    /// Allocates a new resource without initializing it
    static GfxResource alloc();

    /// Allocates and initializes a new resource
    static GfxResource make(Desc const& desc);

    GfxResource() = default;
    GfxResource(GfxResource&& other) noexcept : handle_{other.handle_} { other.handle_ = {}; }

    GfxResource& operator=(GfxResource&& other) noexcept
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

    /// Initializes the resource. If the resource doesn't have a valid handle, this will allocate
    /// one. If the resource has already been initialized, this will deinitialize it first.
    void init(Desc const& desc);

    /// True if the resource has a valid handle
    bool is_valid() const { return handle_.id != SG_INVALID_ID; }

    /// True if the resource has been initialized
    bool is_init() const
    {
        auto const rs = status();
        return rs == SG_RESOURCESTATE_VALID || rs == SG_RESOURCESTATE_FAILED;
    }

    /// Returns the current status of the resource
    sg_resource_state status() const;

  private:
    Handle handle_{};

    GfxResource(Handle const handle) : handle_{handle} {}

    void destroy();
};

using GfxPipeline = GfxResource<sg_pipeline>;
using GfxShader = GfxResource<sg_shader>;
using GfxBuffer = GfxResource<sg_buffer>;
using GfxImage = GfxResource<sg_image>;
using GfxSampler = GfxResource<sg_sampler>;
using GfxView = GfxResource<sg_view>;

template <>
struct Traits<sg_pipeline>
{
    using Desc = sg_pipeline_desc;
};

template <>
struct Traits<sg_shader>
{
    using Desc = sg_shader_desc;
};

template <>
struct Traits<sg_buffer>
{
    using Desc = sg_buffer_desc;
};

template <>
struct Traits<sg_image>
{
    using Desc = sg_image_desc;
};

template <>
struct Traits<sg_sampler>
{
    using Desc = sg_sampler_desc;
};

template <>
struct Traits<sg_view>
{
    using Desc = sg_view_desc;
};

} // namespace dr