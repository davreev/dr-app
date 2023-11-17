#include <dr/app/camera_utils.hpp>

namespace dr
{

Camera make_camera(Orbit const& orbit, Zoom const& zoom)
{
    Camera cam{};
    orbit.apply(cam);
    zoom.apply(cam);
    return cam;
}

Camera make_camera_front(Vec3<f32> const& target, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = Vec3<f32>::UnitX();

    Camera cam;
    cam.pivot.rotation.q = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

Camera make_camera_back(Vec3<f32> const& target, f32 const offset)
{
    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = -Vec3<f32>::UnitX();

    Camera cam;
    cam.pivot.rotation.q = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

Camera make_camera_look_at(
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

    Camera cam;
    cam.pivot.rotation.q = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = d_norm;
    return cam;
}

void frame_bounds(Camera& camera, Vec3<f32> const& center, f32 const radius, f32 const fov_min)
{
    camera.pivot.position = center;
    camera.offset = {0.0f, 0.0f, radius / std::sin(fov_min * 0.5f)};
}

void transition(Camera& camera, Camera const& target, f32 const t)
{
    camera.offset += (target.offset - camera.offset) * t;
    camera.pivot.position += (target.pivot.position - camera.pivot.position) * t;
    camera.pivot.rotation.q = camera.pivot.rotation.q.slerp(t, target.pivot.rotation.q);
}

} // namespace dr