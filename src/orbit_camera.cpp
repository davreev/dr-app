#include <dr/app/orbit_camera.hpp>

#include <dr/app/gfx_utils.hpp>

namespace dr
{

OrbitCamera::OrbitCamera()
{
    controls.orbit.apply(rig);
    controls.zoom.apply(rig);
    controls.pan.apply(rig);
}

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

void OrbitCamera::frame_target()
{
    controls.zoom.distance.target = target.radius / std::sin(frustum.fov_y * 0.5);
    controls.pan.offset.target = {};
}

void OrbitCamera::frame_target_now()
{
    frame_target();
    controls.zoom.distance.current = controls.zoom.distance.target;
    controls.pan.offset.current = {};
}

Mat4<f32> OrbitCamera::make_world_to_view() const { return rig.transform().inverse_to_matrix(); }

Mat4<f32> OrbitCamera::make_view_to_clip(f32 const aspect) const
{
    return make_perspective<NdcType_OpenGl>(
        frustum.fov_y,
        aspect,
        frustum.clip_near,
        frustum.clip_far);
}

} // namespace dr