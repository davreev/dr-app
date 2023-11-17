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
    // Vec2<f32> min_offset{};
    // Vec2<f32> max_offset{};
    f32 sensitivity{1.0};

    void apply(Camera& camera) const { camera.offset.head<2>() = offset; }

    void handle_drag(Vec2<f32> const& delta)
    {
        constexpr f32 dir_x{-1.0};
        constexpr f32 dir_y{1.0};
        offset.x() += dir_x * delta.x() * sensitivity;
        offset.y() += dir_y * delta.y() * sensitivity;
        // TODO(dr): Clamp offsets?
    }
};

struct Zoom
{
    f32 distance{1.0};
    // f32 min_distance{};
    // f32 max_distance{};
    f32 sensitivity{1.0};

    void apply(Camera& camera) const { camera.offset.z() = distance; }

    void handle_scroll(f32 const delta)
    {
        constexpr f32 dir{-1.0};
        distance += dir * delta * sensitivity;
        // TODO(dr): Clamp distance?
    }
};

struct Orbit
{
    f32 polar{};
    f32 azimuth{};
    // Vec2<f32> min_azimuth{};
    // Vec2<f32> max_azimuth{};
    f32 sensitivity{5.0};

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
        // TODO(dr): Clamp azimuth?
    }
};

struct EasedPan
{
    Pan current;
    Pan target;

    EasedPan(Pan const& pan) : current{pan}, target{pan} {}

    void apply(Camera& camera) const { current.apply(camera); }

    void update(f32 const t) { current.offset += (target.offset - current.offset) * t; }
};

struct EasedZoom
{
    Zoom current;
    Zoom target;

    EasedZoom(Zoom const& zoom) : current{zoom}, target{zoom} {}

    void apply(Camera& camera) const { current.apply(camera); }

    void update(f32 const t) { current.distance += (target.distance - current.distance) * t; };
};

struct EasedOrbit
{
    Orbit current;
    Orbit target;

    EasedOrbit(Orbit const& orbit) : current{orbit}, target{orbit} {}

    void apply(Camera& camera) const { current.apply(camera); }

    void update(f32 const t)
    {
        current.polar += (target.polar - current.polar) * t;
        current.azimuth += (target.azimuth - current.azimuth) * t;
    };
};

} // namespace dr