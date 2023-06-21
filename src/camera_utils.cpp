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
    Camera cam{};

    Mat3<f32> m;
    m.col(0) = Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = Vec3<f32>::UnitX();

    cam.pivot.rotation = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

Camera make_camera_back(Vec3<f32> const& target, f32 const offset)
{
    Camera cam{};

    Mat3<f32> m;
    m.col(0) = -Vec3<f32>::UnitY();
    m.col(1) = Vec3<f32>::UnitZ();
    m.col(2) = -Vec3<f32>::UnitX();

    cam.pivot.rotation = Quat<f32>{m};
    cam.pivot.position = target;
    cam.offset.z() = offset;
    return cam;
}

Camera make_camera_look_at(
    Vec3<f32> const& position,
    Vec3<f32> const& target,
    Vec3<f32> const& up)
{
    Camera cam{};
    cam.pivot.position = target;

    Vec3<f32> const d = position - target;
    f32 const d_norm = d.norm();

    // Create pivot rotation
    Mat3<f32> m;
    m.col(2) = d / d_norm;
    m.col(0) = up.cross(m.col(2)).normalized();
    m.col(1) = m.col(2).cross(m.col(0));

    // Assign rotation and offset
    cam.pivot.rotation = Quat<f32>{m};
    cam.offset.z() = d_norm;

    return cam;
}

void frame_bounds(Camera& camera, Vec3<f32> const& center, f32 const radius, f32 const fov_min)
{
    camera.pivot.position = center;
    camera.offset = {0.0f, 0.0f, radius / std::sin(fov_min * 0.5f)};
}

} // namespace dr