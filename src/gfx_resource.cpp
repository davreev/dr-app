#include <dr/app/gfx_resource.hpp>

namespace dr
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxPipeline

template <>
GfxPipeline GfxPipeline::alloc()
{
    return {sg_alloc_pipeline()};
}

template <>
GfxPipeline GfxPipeline::make(Desc const& desc)
{
    return {sg_make_pipeline(desc)};
}

template <>
void GfxPipeline::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_pipeline(handle_);
        handle_ = {};
    }
}

template <>
void GfxPipeline::init(GfxPipeline::Desc const& desc)
{
    if (is_init())
        sg_uninit_pipeline(handle_);

    sg_init_pipeline(handle_, desc);
}

template <>
sg_resource_state GfxPipeline::query_state() const
{
    return sg_query_pipeline_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxShader

template <>
GfxShader GfxShader::alloc()
{
    return {sg_alloc_shader()};
}

template <>
GfxShader GfxShader::make(Desc const& desc)
{
    return {sg_make_shader(desc)};
}

template <>
void GfxShader::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_shader(handle_);
        handle_ = {};
    }
}

template <>
void GfxShader::init(GfxShader::Desc const& desc)
{
    if (is_init())
        sg_uninit_shader(handle_);

    sg_init_shader(handle_, desc);
}

template <>
sg_resource_state GfxShader::query_state() const
{
    return sg_query_shader_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxBuffer

template <>
GfxBuffer GfxBuffer::alloc()
{
    return {sg_alloc_buffer()};
}

template <>
GfxBuffer GfxBuffer::make(Desc const& desc)
{
    return {sg_make_buffer(desc)};
}

template <>
void GfxBuffer::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_buffer(handle_);
        handle_ = {};
    }
}

template <>
void GfxBuffer::init(GfxBuffer::Desc const& desc)
{
    if (is_init())
        sg_uninit_buffer(handle_);

    sg_init_buffer(handle_, desc);
}

template <>
sg_resource_state GfxBuffer::query_state() const
{
    return sg_query_buffer_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxImage

template <>
GfxImage GfxImage::alloc()
{
    return {sg_alloc_image()};
}

template <>
GfxImage GfxImage::make(Desc const& desc)
{
    return {sg_make_image(desc)};
}

template <>
void GfxImage::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_image(handle_);
        handle_ = {};
    }
}

template <>
void GfxImage::init(GfxImage::Desc const& desc)
{
    if (is_init())
        sg_uninit_image(handle_);

    sg_init_image(handle_, desc);
}

template <>
sg_resource_state GfxImage::query_state() const
{
    return sg_query_image_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxSampler

template <>
GfxSampler GfxSampler::alloc()
{
    return {sg_alloc_sampler()};
}

template <>
GfxSampler GfxSampler::make(Desc const& desc)
{
    return {sg_make_sampler(desc)};
}

template <>
void GfxSampler::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_sampler(handle_);
        handle_ = {};
    }
}

template <>
void GfxSampler::init(GfxSampler::Desc const& desc)
{
    if (is_init())
        sg_uninit_sampler(handle_);

    sg_init_sampler(handle_, desc);
}

template <>
sg_resource_state GfxSampler::query_state() const
{
    return sg_query_sampler_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxView

template <>
GfxView GfxView::alloc()
{
    return {sg_alloc_view()};
}

template <>
GfxView GfxView::make(Desc const& desc)
{
    return {sg_make_view(desc)};
}

template <>
void GfxView::destroy()
{
    if (is_valid() && sg_isvalid())
    {
        sg_destroy_view(handle_);
        handle_ = {};
    }
}

template <>
void GfxView::init(GfxView::Desc const& desc)
{
    if (is_init())
        sg_uninit_view(handle_);

    sg_init_view(handle_, desc);
}

template <>
sg_resource_state GfxView::query_state() const
{
    return sg_query_view_state(handle_);
}

} // namespace dr