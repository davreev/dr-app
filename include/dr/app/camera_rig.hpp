#pragma once

#include <dr/linalg_reshape.hpp>
#include <dr/math.hpp>
#include <dr/transform.hpp>

namespace dr
{

struct CameraRig
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
    struct
    {
        Vec2<f32> current{};
        Vec2<f32> target{current};
    } offset{};
    f32 sensitivity{1.0};

    void set(CameraRig const& rig);
    void set_target(CameraRig const& rig) { offset.target = rig.offset.head<2>(); }
    void apply(CameraRig& rig) const { rig.offset.head<2>() = offset.current; }
    void update(f32 const t) { offset.current += (offset.target - offset.current) * t; }
    void handle_input(Vec2<f32> const& delta);
};

struct Zoom
{
    struct
    {
        f32 current{1.0};
        f32 target{current};
        f32 min{};
        f32 max{1.0e5};
    } distance{};
    f32 sensitivity{1.0};

    void set(CameraRig const& rig);
    void set_target(CameraRig const& rig) { distance.target = rig.offset.z(); }
    void apply(CameraRig& rig) const { rig.offset.z() = distance.current; }
    void update(f32 const t) { distance.current += (distance.target - distance.current) * t; }
    void handle_input(f32 delta);
};

struct Orbit
{
    struct
    {
        f32 current{pi<f32> * 0.5f};
        f32 target{current};
        f32 min{};
        f32 max{pi<f32>};
    } polar{};
    struct
    {
        f32 current{};
        f32 target{current};
    } azimuth{};
    f32 sensitivity{5.0};

    void set(CameraRig const& rig);
    void set_target(CameraRig const& rig);
    void apply(CameraRig& rig) const;
    void update(f32 t);
    void handle_input(Vec2<f32> const& delta);
};

/// Creates a camera rig from the given controls
CameraRig make_camera(Orbit const& orbit, Zoom const& zoom);

/// Creates a camera rig from the given controls
CameraRig make_camera(Orbit const& orbit, Zoom const& zoom, Pan const& pan);

/// Creates a camera rig looking down the x axis at a given target
CameraRig make_camera_front(Vec3<f32> const& target, f32 offset);

/// Creates a camera rig looking up the x axis at a given target
CameraRig make_camera_back(Vec3<f32> const& target, f32 offset);

/// Creates a camera rig located at a given position looking at a given target
CameraRig make_camera_look_at(
    Vec3<f32> const& position,
    Vec3<f32> const& target,
    Vec3<f32> const& up);

/// Translates the camera rig to fit a given sphere in view
void camera_frame_bounds(CameraRig& rig, Vec3<f32> const& center, f32 radius, f32 fov_min);

/// Transitions the camera rig to another
void camera_transition(CameraRig& rig, CameraRig const& target, f32 const t);

} // namespace dr