#pragma once

#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/transform.hpp>

namespace dr
{

struct Camera
{
    /// Position and rotation of the pivot in world space
    struct
    {
        Rotation3<f32> rotation{};
        Vec3<f32> position{};
    } pivot;

    /// Offset in pivot space
    Vec3<f32> offset{Vec3<f32>::UnitZ()};

    /// Returns the position of the camera in world space
    Vec3<f32> position() const { return pivot.rotation * offset + pivot.position; }

    /// Returns the transformation from local space to world space
    Rigid3<f32> transform() const
    {
        return {
            pivot.rotation,
            pivot.position + pivot.rotation * offset,
        };
    }
};

struct Pan
{
    Vec2<f32> offset{};
    f32 sensitivity{1.0};

    void set(Camera const& camera) { offset = camera.offset.head<2>(); }

    void apply(Camera& camera) const { camera.offset.head<2>() = offset; }

    void handle_input(Vec2<f32> const& delta)
    {
        constexpr f32 dir_x{-1.0};
        constexpr f32 dir_y{1.0};
        offset.x() += dir_x * delta.x() * sensitivity;
        offset.y() += dir_y * delta.y() * sensitivity;
    }
};

struct Zoom
{
    f32 distance{1.0};
    f32 sensitivity{1.0};
    f32 min_distance{0.0};
    f32 max_distance{1000.0};

    void set(Camera const& camera) { distance = camera.offset.z(); }

    void apply(Camera& camera) const { camera.offset.z() = distance; }

    void handle_input(f32 const delta)
    {
        constexpr f32 dir{-1.0};
        distance += dir * delta * sensitivity;
        distance = clamp(distance, min_distance, max_distance);
    }
};

struct Orbit
{
    f32 polar{};
    f32 azimuth{};
    f32 sensitivity{5.0};
    f32 min_polar{0.0f};
    f32 max_polar{pi<f32>};

    void set(Camera const& camera)
    {
        Mat3<f32> const piv_rot = camera.pivot.rotation.to_matrix();
        Vec3<f32> const rx = piv_rot.col(0);
        Vec3<f32> const rz = piv_rot.col(2);

        polar = acos_safe(rz.z());
        azimuth = std::atan2(rx.y(), rx.x());
    }

    void apply(Camera& camera) const
    {
        constexpr f32 up[]{0.0, 0.0, 1.0};
        constexpr f32 right[]{1.0, 0.0, 0.0};

        Eigen::AngleAxis const r_z{azimuth, as_vec<3>(up)};
        Eigen::AngleAxis const r_x{polar, as_vec<3>(right)};
        camera.pivot.rotation.q = r_z * r_x;
    }

    void handle_input(Vec2<f32> const& delta)
    {
        constexpr f32 dir_x{-1.0};
        constexpr f32 dir_y{-1.0};

        azimuth += dir_x * delta.x() * sensitivity;
        polar += dir_y * delta.y() * sensitivity;
        polar = clamp(polar, min_polar, max_polar);
    }
};

/// Creates a camera from the given controls
Camera make_camera(Orbit const& orbit, Zoom const& zoom);

/// Creates a camera looking down the x axis at a given target
Camera make_camera_front(Vec3<f32> const& target, f32 offset);

/// Creates a camera looking up the x axis at a given target
Camera make_camera_back(Vec3<f32> const& target, f32 offset);

/// Creates a camera located at a given position looking at a given target
Camera make_camera_look_at(Vec3<f32> const& position, Vec3<f32> const& target, Vec3<f32> const& up);

/// Translates the camera to fit a given sphere in view
void camera_frame_bounds(Camera& camera, Vec3<f32> const& center, f32 radius, f32 fov_min);

/// Transitions the camera to another
void camera_transition(Camera& camera, Camera const& target, f32 const t);

} // namespace dr