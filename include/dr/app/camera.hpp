#pragma once

#include <dr/math.hpp>

namespace dr
{

struct Camera
{
    /// Position and rotation of the pivot in world space
    struct
    {
        Quat<f32> rotation{Quat<f32>::Identity()};
        Vec3<f32> position{Vec3<f32>::Zero()};
    } pivot;

    /// Offset in pivot space
    Vec3<f32> offset{Vec3<f32>::UnitZ()};

    /// Returns the position of the camera in world space.
    Vec3<f32> position() const
    {
        return pivot.rotation * offset + pivot.position;
    }

    /// Returns the transformation from world space to local space. This assumes the current pivot
    /// rotation is unit length.
    Mat4<f32> world_to_local() const
    {
        Mat4<f32> m;
        m.setIdentity();

        auto r = m.topLeftCorner<3, 3>();
        r = pivot.rotation.toRotationMatrix().transpose();

        auto t = m.topRightCorner<3, 1>();
        t = -offset - r * pivot.position;

        return m;
    }

    /// Returns the transformation from local space to world space. This assumes the current pivot
    /// rotation is unit length.
    Mat4<f32> local_to_world() const
    {
        Mat4<f32> m;
        m.setIdentity();

        auto r = m.topLeftCorner<3, 3>();
        r = pivot.rotation.toRotationMatrix();

        auto t = m.topRightCorner<3, 1>();
        t = r * offset + pivot.position;

        return m;
    }
};

struct Pan
{
    Vec2<f32> offset{};
    // Vec2<f32> min_offset{};
    // Vec2<f32> max_offset{};
    f32 sensitivity{1.0f};

    void apply(Camera& camera) const
    {
        camera.offset.head<2>() = offset;
    }

    void handle_drag(Camera& camera, Vec2<f32> const& delta)
    {
        // TODO(dr): Expose these?
        constexpr f32 dir_x{-1.0f};
        constexpr f32 dir_y{1.0f};

        // Change in xy offset is proportional to the current z offset
        f32 const scale = camera.offset.z() * sensitivity;
        offset.x() += dir_x * delta.x() * scale;
        offset.y() += dir_y * delta.y() * scale;

        // TODO(dr): Clamp offset?
        apply(camera);
    }
};

struct Zoom
{
    f32 distance{1.0f};
    // f32 min_distance{1.0f};
    // f32 max_distance{1000.0f};
    f32 sensitivity{1.0f};

    void apply(Camera& camera) const
    {
        camera.offset.z() = distance;
    }

    void handle_scroll(Camera& camera, f32 const delta)
    {
        // TODO(dr): Expose this?
        constexpr f32 dir{-1.0f};

        // Change in distance is proportional to the current z offset
        f32 const scale = camera.offset.z() * sensitivity;
        distance += dir * delta * scale;

        // TODO(dr): Clamp distance?
        apply(camera);
    }
};

struct Orbit
{
    f32 polar{};
    f32 azimuth{};
    // Vec2<f32> min_azimuth{};
    // Vec2<f32> max_azimuth{};
    f32 sensitivity{5.0f};

    void apply(Camera& camera) const
    {
        // TODO(dr): Make these globals for compatibility w different conventions
        static Vec3<f32> const up = Vec3<f32>::UnitZ();
        static Vec3<f32> const right = Vec3<f32>::UnitX();

        Quat<f32> rx, rz;
        rz = Eigen::AngleAxis{polar, up};
        rx = Eigen::AngleAxis{azimuth, right};
        camera.pivot.rotation = rz * rx;
    }

    void handle_drag(Camera& camera, Vec2<f32> const& delta)
    {
        // TODO(dr): Expose these?
        constexpr f32 dir_x{-1.0f};
        constexpr f32 dir_y{-1.0f};

        polar += dir_x * delta.x() * sensitivity;
        azimuth += dir_y * delta.y() * sensitivity;

        // TODO(dr): Clamp azimuth?
        apply(camera);
    }
};

struct SmoothCamera
{
    Camera current;
    Camera target;
    f32 sensitivity;

    SmoothCamera(Camera const& init, f32 const sensitivity = 10.0f) :
        current{init}, target{init}, sensitivity{sensitivity} {}

    void update(f32 const delta_time)
    {
        f32 const t = saturate(sensitivity * delta_time);
        current.offset += (target.offset - current.offset) * t;
        current.pivot.position += (target.pivot.position - current.pivot.position) * t;
        current.pivot.rotation = current.pivot.rotation.slerp(t, target.pivot.rotation);
    }
};

} // namespace dr