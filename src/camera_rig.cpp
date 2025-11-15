#include <dr/app/camera_rig.hpp>

namespace dr
{
namespace
{

CameraRig make_camera_rig(
    Rotation3<f32> const& pivot_rot,
    Vec3<f32> const& pivot_pos,
    f32 const offset)
{
    return {{pivot_rot, pivot_pos}, {0.0, 0.0, offset}};
}

} // namespace

void CameraRig::frame_bounds(
    CameraRig& rig,
    Vec3<f32> const& center,
    f32 const radius,
    f32 const fov_min)
{
    rig.pivot.position = center;
    rig.offset = {0.0f, 0.0f, radius / std::sin(fov_min * 0.5f)};
}

void CameraRig::transition_to(CameraRig const& other, f32 const t)
{
    offset += (other.offset - offset) * t;
    pivot.position += (other.pivot.position - pivot.position) * t;
    pivot.rotation.q = pivot.rotation.q.slerp(t, other.pivot.rotation.q);
}

CameraRig CameraRig::make_front(Vec3<f32> const& origin, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitX();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = -Vec3<f32>::UnitY();

    return make_camera_rig({Quat<f32>{m}}, origin, offset);
}

CameraRig CameraRig::make_back(Vec3<f32> const& origin, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitX();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = Vec3<f32>::UnitY();

    return make_camera_rig({Quat<f32>{m}}, origin, offset);
}

CameraRig CameraRig::make_right(Vec3<f32> const& origin, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = Vec3<f32>::UnitX();

    return make_camera_rig({Quat<f32>{m}}, origin, offset);
}

CameraRig CameraRig::make_left(Vec3<f32> const& origin, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = -Vec3<f32>::UnitX();

    return make_camera_rig({Quat<f32>{m}}, origin, offset);
}

CameraRig CameraRig::make_top(Vec3<f32> const& origin, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitX();
    m.col(1) = Vec3<f32>::UnitY();
    m.col(2) = Vec3<f32>::UnitZ();

    return make_camera_rig({Quat<f32>{m}}, origin, offset);
}

CameraRig CameraRig::make_bottom(Vec3<f32> const& origin, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitX();
    m.col(1) = Vec3<f32>::UnitY();
    m.col(2) = -Vec3<f32>::UnitZ();

    return make_camera_rig({Quat<f32>{m}}, origin, offset);
}

CameraRig CameraRig::make_look_at(
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

    return make_camera_rig({Quat<f32>{m}}, target, d_norm);
}

} // namespace dr