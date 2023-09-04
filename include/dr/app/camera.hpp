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
    Vec3<f32> position() const
    {
        return pivot.rotation * offset + pivot.position;
    }

    /// Returns the transformation from local space to world space
    Rigid3<f32> transform() const
    {
        return {
            pivot.rotation,
            pivot.position + pivot.rotation * offset,
        };
    }

    /// Transitions the camera to another
    void transition_to(Camera const& other, f32 const t)
    {
        offset += (other.offset - offset) * t;
        pivot.position += (other.pivot.position - pivot.position) * t;
        pivot.rotation.q = pivot.rotation.q.slerp(t, other.pivot.rotation.q);
    }
};

struct Pan
{
    Vec2<f32> offset{};
    // Vec2<f32> min_offset{};
    // Vec2<f32> max_offset{};
    f32 sensitivity{1.0};

    void apply(Camera& camera) const
    {
        camera.offset.head<2>() = offset;
    }

    void handle_drag(Camera& camera, Vec2<f32> const& delta)
    {
        constexpr f32 dir_x{-1.0};
        constexpr f32 dir_y{1.0};

        // Change in xy offset is proportional to the current z offset
        f32 const scale = camera.offset.z() * sensitivity;
        offset.x() += dir_x * delta.x() * scale;
        offset.y() += dir_y * delta.y() * scale;

        // TODO(dr): Clamp offsets

        apply(camera);
    }
};

struct Zoom
{
    f32 distance{1.0};
    // f32 min_distance{};
    // f32 max_distance{};
    f32 sensitivity{1.0};

    void apply(Camera& camera) const
    {
        camera.offset.z() = distance;
    }

    void handle_scroll(Camera& camera, f32 const delta)
    {
        constexpr f32 dir{-1.0};

        // Change in distance is proportional to the current z offset
        f32 const scale = camera.offset.z() * sensitivity;
        distance += dir * delta * scale;

        // TODO(dr): Clamp distance

        apply(camera);
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

    void handle_drag(Camera& camera, Vec2<f32> const& delta)
    {
        constexpr f32 dir_x{-1.0};
        constexpr f32 dir_y{-1.0};

        polar += dir_x * delta.x() * sensitivity;
        azimuth += dir_y * delta.y() * sensitivity;

        // TODO(dr): Clamp azimuth

        apply(camera);
    }
};

} // namespace dr