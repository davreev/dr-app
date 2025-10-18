#include <dr/app/camera_rig.hpp>

namespace dr
{

CameraRig make_camera_front(Vec3<f32> const& target, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = Vec3<f32>::UnitX();

    CameraRig cam;
    cam.pivot.rotation.q = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

CameraRig make_camera_back(Vec3<f32> const& target, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = -Vec3<f32>::UnitX();

    CameraRig cam;
    cam.pivot.rotation.q = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

CameraRig make_camera_look_at(
    Vec3<f32> const& position,
    Vec3<f32> const& target,
    Vec3<f32> const& up)
{
    Vec3<f32> const d = position - target;
    f32 const d_norm = d.norm();

    Mat3<f32> m;
    m.col(2) = d / d_norm;
    m.col(0) = up.cross(m.col(2)).normalized();
    m.col(1) = m.col(2).cross(m.col(0));

    CameraRig cam;
    cam.pivot.rotation.q = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = d_norm;
    return cam;
}

void camera_frame_bounds(
    CameraRig& rig,
    Vec3<f32> const& center,
    f32 const radius,
    f32 const fov_min)
{
    rig.pivot.position = center;
    rig.offset = {0.0f, 0.0f, radius / std::sin(fov_min * 0.5f)};
}

void camera_transition(CameraRig& rig, CameraRig const& target, f32 const t)
{
    rig.offset += (target.offset - rig.offset) * t;
    rig.pivot.position += (target.pivot.position - rig.pivot.position) * t;
    rig.pivot.rotation.q = rig.pivot.rotation.q.slerp(t, target.pivot.rotation.q);
}

} // namespace dr