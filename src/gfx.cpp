#include <dr/app/gfx.hpp>

#include <cassert>

namespace dr
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxPass

template <>
GfxPass::GfxResource(GfxPass::Desc const& desc) :
    handle_{sg_make_pass(desc)}
{
}

template <>
void GfxPass::alloc()
{
    assert(!is_alloc());
    handle_ = sg_alloc_pass();
}

template <>
void GfxPass::destroy()
{
    if (is_alloc() && sg_isvalid())
    {
        sg_destroy_pass(handle_);
        handle_ = {};
    }
}

template <>
void GfxPass::init(GfxPass::Desc const& desc)
{
    if (!is_alloc())
    {
        handle_ = sg_make_pass(desc);
    }
    else
    {
        if (is_init())
            sg_uninit_pass(handle_);

        sg_init_pass(handle_, desc);
    }
}

template <>
GfxPass::State GfxPass::query_state() const
{
    return sg_query_pass_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxPipeline

template <>
GfxPipeline::GfxResource(GfxPipeline::Desc const& desc) :
    handle_{sg_make_pipeline(desc)}
{
}

template <>
void GfxPipeline::alloc()
{
    assert(!is_alloc());
    handle_ = sg_alloc_pipeline();
}

template <>
void GfxPipeline::destroy()
{
    if (is_alloc() && sg_isvalid())
    {
        sg_destroy_pipeline(handle_);
        handle_ = {};
    }
}

template <>
void GfxPipeline::init(GfxPipeline::Desc const& desc)
{
    if (!is_alloc())
    {
        handle_ = sg_make_pipeline(desc);
    }
    else
    {
        if (is_init())
            sg_uninit_pipeline(handle_);

        sg_init_pipeline(handle_, desc);
    }
}

template <>
GfxPipeline::State GfxPipeline::query_state() const
{
    return sg_query_pipeline_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxShader

template <>
GfxShader::GfxResource(GfxShader::Desc const& desc) :
    handle_{sg_make_shader(desc)}
{
}

template <>
void GfxShader::alloc()
{
    assert(!is_alloc());
    handle_ = sg_alloc_shader();
}

template <>
void GfxShader::destroy()
{
    if (is_alloc() && sg_isvalid())
    {
        sg_destroy_shader(handle_);
        handle_ = {};
    }
}

template <>
void GfxShader::init(GfxShader::Desc const& desc)
{
    if (!is_alloc())
    {
        handle_ = sg_make_shader(desc);
    }
    else
    {
        if (is_init())
            sg_uninit_shader(handle_);

        sg_init_shader(handle_, desc);
    }
}

template <>
GfxShader::State GfxShader::query_state() const
{
    return sg_query_shader_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxBuffer

template <>
GfxBuffer::GfxResource(GfxBuffer::Desc const& desc) :
    handle_{sg_make_buffer(desc)}
{
}

template <>
void GfxBuffer::alloc()
{
    assert(!is_alloc());
    handle_ = sg_alloc_buffer();
}

template <>
void GfxBuffer::destroy()
{
    if (is_alloc() && sg_isvalid())
    {
        sg_destroy_buffer(handle_);
        handle_ = {};
    }
}

template <>
void GfxBuffer::init(GfxBuffer::Desc const& desc)
{
    if (!is_alloc())
    {
        handle_ = sg_make_buffer(desc);
    }
    else
    {
        if (is_init())
            sg_uninit_buffer(handle_);

        sg_init_buffer(handle_, desc);
    }
}

template <>
GfxBuffer::State GfxBuffer::query_state() const
{
    return sg_query_buffer_state(handle_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GfxImage

template <>
GfxImage::GfxResource(GfxImage::Desc const& desc) :
    handle_{sg_make_image(desc)}
{
}

template <>
void GfxImage::alloc()
{
    assert(!is_alloc());
    handle_ = sg_alloc_image();
}

template <>
void GfxImage::destroy()
{
    if (is_alloc() && sg_isvalid())
    {
        sg_destroy_image(handle_);
        handle_ = {};
    }
}

template <>
void GfxImage::init(GfxImage::Desc const& desc)
{
    if (!is_alloc())
    {
        handle_ = sg_make_image(desc);
    }
    else
    {
        if (is_init())
            sg_uninit_image(handle_);

        sg_init_image(handle_, desc);
    }
}

template <>
GfxImage::State GfxImage::query_state() const
{
    return sg_query_image_state(handle_);
}

} // namespace dr