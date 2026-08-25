#pragma once

#include <dr/basic_types.hpp>

namespace dr
{

enum NdcType : u8
{
    NdcType_Default = 0, // z in [0, 1], y up
    NdcType_OpenGl, // z in [-1, 1], y up
    NdcType_Vulkan // z in [0, 1], y down
};

#if defined(SOKOL_GLCORE) || defined(SOKOL_GLES3)
inline constexpr NdcType default_ndc = NdcType_OpenGl;
#elif defined(SOKOL_D3D11) || defined(SOKOL_METAL) || defined(SOKOL_WGPU)
inline constexpr NdcType default_ndc = NdcType_Default;
#elif defined(SOKOL_VULKAN)
inline constexpr NdcType default_ndc = NdcType_Vulkan;
#else
#error "Sokol backend not selected"
#endif

} // namespace dr