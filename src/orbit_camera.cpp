#include <dr/app/orbit_camera.hpp>

#include <cassert>

#include <dr/math.hpp>

#include <dr/app/gfx_utils.hpp>

namespace dr
{

OrbitCamera::OrbitCamera()
{
    controls.orbit.apply(rig);
    controls.zoom.apply(rig);
    controls.pan.apply(rig);
    rig.pivot.position = target.position;
}

OrbitCamera::OrbitCamera(CameraRig const& rig) { set_rig_now(rig); }

void OrbitCamera::update(f64 const delta_time_sec)
{
    f32 const t = saturate(controls.stiffness * delta_time_sec);

    controls.orbit.update(t);
    controls.orbit.apply(rig);

    controls.zoom.update(t);
    controls.zoom.apply(rig);

    controls.pan.update(t);
    controls.pan.apply(rig);

    rig.pivot.position += (target.position - rig.pivot.position) * t;
}

void OrbitCamera::set_rig(CameraRig const& value)
{
    controls.orbit.set_target(value);
    controls.zoom.set_target(value);
    controls.pan.set_target(value);
    target.position = value.pivot.position;
}

void OrbitCamera::set_rig_now(CameraRig const& value)
{
    rig = value;
    controls.orbit.set(value);
    controls.zoom.set(value);
    controls.pan.set(value);
    target.position = value.pivot.position;
}

void OrbitCamera::frame_target()
{
    controls.zoom.distance.target = target.radius / std::sin(frustum.fov_y * 0.5);
    controls.pan.offset.target = {};
}

void OrbitCamera::frame_target_now()
{
    frame_target();
    controls.zoom.distance.current = controls.zoom.distance.target;
    controls.pan.offset.current = controls.pan.offset.target;
}

Mat4<f32> OrbitCamera::make_world_to_view() const { return rig.transform().inverse_to_matrix(); }

template <NdcType ndc>
Mat4<f32> OrbitCamera::make_view_to_clip(f32 const aspect) const
{
    switch (projection)
    {
        case Projection_Perspective:
        {
            return make_perspective<ndc>(
                frustum.fov_y,
                aspect,
                frustum.clip_near,
                frustum.clip_far);
        }
        case Projection_Orthographic:
        {
            // NOTE(dr): Using fov_y to establish the height of the ortho frustum keeps scale
            // consistent when switching between projection modes
            return make_orthographic<ndc>(
                2.0f * std::tan(0.5f * frustum.fov_y) * controls.zoom.distance.current,
                aspect,
                -frustum.clip_far,
                frustum.clip_far);
        }
        default:
        {
            assert(false);
            return Mat4<f32>::Identity();
        }
    }
}

template Mat4<f32> OrbitCamera::make_view_to_clip<NdcType_Default>(f32) const;
template Mat4<f32> OrbitCamera::make_view_to_clip<NdcType_OpenGl>(f32) const;
template Mat4<f32> OrbitCamera::make_view_to_clip<NdcType_Vulkan>(f32) const;

} // namespace dr