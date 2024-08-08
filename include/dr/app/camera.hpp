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

    void apply(Camera& camera) const { camera.offset.head<2>() = offset; }

    void handle_drag(Vec2<f32> const& delta)
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

    void apply(Camera& camera) const { camera.offset.z() = distance; }

    void handle_scroll(f32 const delta)
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
    f32 min_azimuth{0.0f};
    f32 max_azimuth{pi<f32>};

    void apply(Camera& camera) const
    {
        constexpr f32 up[]{0.0, 0.0, 1.0};
        constexpr f32 right[]{1.0, 0.0, 0.0};

        Eigen::AngleAxis const r_z{polar, as_vec<3>(up)};
        Eigen::AngleAxis const r_x{azimuth, as_vec<3>(right)};
        camera.pivot.rotation.q = r_z * r_x;
    }

    void handle_drag(Vec2<f32> const& delta)
    {
        constexpr f32 dir_x{-1.0};
        constexpr f32 dir_y{-1.0};
        polar += dir_x * delta.x() * sensitivity;
        azimuth += dir_y * delta.y() * sensitivity;
        azimuth = clamp(azimuth, min_azimuth, max_azimuth);
    }
};

struct EasedPan
{
    Pan current;
    Pan target;

    EasedPan(Pan const& pan = {}) : current{pan}, target{pan} {}

    void apply(Camera& camera) const { current.apply(camera); }

    void update(f32 const t) { current.offset += (target.offset - current.offset) * t; }
};

struct EasedZoom
{
    Zoom current;
    Zoom target;

    EasedZoom(Zoom const& zoom = {}) : current{zoom}, target{zoom} {}

    void apply(Camera& camera) const { current.apply(camera); }

    void update(f32 const t) { current.distance += (target.distance - current.distance) * t; };
};

struct EasedOrbit
{
    Orbit current;
    Orbit target;

    EasedOrbit(Orbit const& orbit = {}) : current{orbit}, target{orbit} {}

    void apply(Camera& camera) const { current.apply(camera); }

    void update(f32 const t)
    {
        current.polar += (target.polar - current.polar) * t;
        current.azimuth += (target.azimuth - current.azimuth) * t;
    };
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