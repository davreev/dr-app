#include <dr/app/camera_controls.hpp>

#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>

namespace dr
{

void Pan::set(CameraRig const& rig)
{
    set_target(rig);
    offset.current = offset.target;
}

void Pan::handle_input(Vec2<f32> const& delta)
{
    constexpr f32 dir_x{-1.0};
    constexpr f32 dir_y{1.0};
    offset.target.x() += dir_x * delta.x() * sensitivity;
    offset.target.y() += dir_y * delta.y() * sensitivity;
}

void Zoom::set(CameraRig const& rig)
{
    set_target(rig);
    distance.current = distance.target;
}

void Zoom::handle_input(f32 const delta)
{
    constexpr f32 dir{-1.0};
    distance.target = clamp(
        distance.target + dir * delta * sensitivity,
        distance.min,
        distance.max);
}

void Orbit::set(CameraRig const& rig)
{
    set_target(rig);
    polar.current = polar.target;
    azimuth.current = azimuth.target;
}

void Orbit::set_target(CameraRig const& rig)
{
    Mat3<f32> const piv_rot = rig.pivot.rotation.to_matrix();
    Vec3<f32> const rx = piv_rot.col(0);
    Vec3<f32> const rz = piv_rot.col(2);

    polar.target = acos_safe(rz.z());
    azimuth.target = std::atan2(rx.y(), rx.x());
}

void Orbit::apply(CameraRig& rig) const
{
    constexpr f32 right[]{1.0, 0.0, 0.0};
    constexpr f32 up[]{0.0, 0.0, 1.0};

    Eigen::AngleAxis const r_x{polar.current, as_vec<3>(right)};
    Eigen::AngleAxis const r_z{azimuth.current, as_vec<3>(up)};
    rig.pivot.rotation.q = r_z * r_x;
}

void Orbit::update(f32 const t)
{
    polar.current += (polar.target - polar.current) * t;
    azimuth.current += (azimuth.target - azimuth.current) * t;
}

void Orbit::handle_input(Vec2<f32> const& delta)
{
    constexpr f32 dir_x{-1.0};
    constexpr f32 dir_y{-1.0};

    azimuth.target += dir_x * delta.x() * sensitivity;
    polar.target = clamp(polar.target + dir_y * delta.y() * sensitivity, polar.min, polar.max);
}

CameraRig make_camera(Orbit const& orbit, Zoom const& zoom)
{
    CameraRig cam{};
    orbit.apply(cam);
    zoom.apply(cam);
    return cam;
}

CameraRig make_camera(Orbit const& orbit, Zoom const& zoom, Pan const& pan)
{
    CameraRig cam{};
    orbit.apply(cam);
    zoom.apply(cam);
    pan.apply(cam);
    return cam;
}

} // namespace dr