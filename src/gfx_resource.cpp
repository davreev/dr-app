#include <dr/app/gfx_resource.hpp>

namespace dr
{
namespace
{

template <typename Handle>
Handle sg_alloc_resource();

template <typename Handle>
Handle sg_make_resource(typename Traits<Handle>::Desc const& desc);

template <typename Handle>
void sg_init_resource(Handle handle, typename Traits<Handle>::Desc const& desc);

template <typename Handle>
void sg_uninit_resource(Handle handle);

template <typename Handle>
void sg_destroy_resource(Handle handle);

template <typename Handle>
sg_resource_state sg_query_resource_state(Handle handle);

#define SG_RESOURCE_IMPL(name_)                                                                    \
    template <>                                                                                    \
    sg_##name_ sg_alloc_resource<sg_##name_>()                                                     \
    {                                                                                              \
        return sg_alloc_##name_();                                                                 \
    }                                                                                              \
    template <>                                                                                    \
    sg_##name_ sg_make_resource<sg_##name_>(Traits<sg_##name_>::Desc const& desc)                  \
    {                                                                                              \
        return sg_make_##name_(desc);                                                              \
    }                                                                                              \
    template <>                                                                                    \
    void sg_init_resource<sg_##name_>(sg_##name_ h, Traits<sg_##name_>::Desc const& desc)          \
    {                                                                                              \
        sg_init_##name_(h, desc);                                                                  \
    }                                                                                              \
    template <>                                                                                    \
    void sg_uninit_resource<sg_##name_>(sg_##name_ h)                                              \
    {                                                                                              \
        sg_uninit_##name_(h);                                                                      \
    }                                                                                              \
    template <>                                                                                    \
    void sg_destroy_resource<sg_##name_>(sg_##name_ h)                                             \
    {                                                                                              \
        sg_destroy_##name_(h);                                                                     \
    }                                                                                              \
    template <>                                                                                    \
    sg_resource_state sg_query_resource_state<sg_##name_>(sg_##name_ h)                            \
    {                                                                                              \
        return sg_query_##name_##_state(h);                                                        \
    }

SG_RESOURCE_IMPL(pipeline)
SG_RESOURCE_IMPL(shader)
SG_RESOURCE_IMPL(buffer)
SG_RESOURCE_IMPL(image)
SG_RESOURCE_IMPL(sampler)
SG_RESOURCE_IMPL(view)

#undef SG_RESOURCE_IMPL

} // namespace

template <typename Handle>
GfxResource<Handle> GfxResource<Handle>::alloc()
{
    return {sg_alloc_resource<Handle>()};
}

template <typename Handle>
GfxResource<Handle> GfxResource<Handle>::make(Desc const& desc)
{
    return {sg_make_resource<Handle>(desc)};
}

template <typename Handle>
void GfxResource<Handle>::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_resource<Handle>(handle_);
        handle_ = {};
    }
}

template <typename Handle>
void GfxResource<Handle>::init(Desc const& desc)
{
    if (!is_valid())
    {
        handle_ = sg_make_resource<Handle>(desc);
    }
    else
    {
        if (is_init())
            sg_uninit_resource<Handle>(handle_);
        sg_init_resource<Handle>(handle_, desc);
    }
}

template <typename Handle>
sg_resource_state GfxResource<Handle>::query_state() const
{
    return sg_query_resource_state<Handle>(handle_);
}

// Explicit instantiation for each resource handle type
template struct GfxResource<sg_pipeline>;
template struct GfxResource<sg_shader>;
template struct GfxResource<sg_buffer>;
template struct GfxResource<sg_image>;
template struct GfxResource<sg_sampler>;
template struct GfxResource<sg_view>;

} // namespace dr
