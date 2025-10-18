#pragma once

#include <dr/math_types.hpp>
#include <dr/transform.hpp>

namespace dr
{

struct CameraRig
{
    /// Position and rotation of the pivot in world space
    struct
    {
        Rotation3<f32> rotation;
        Vec3<f32> position;
    } pivot;

    /// Offset of the camera in pivot space
    Vec3<f32> offset{Vec3<f32>::UnitZ()};

    /// Returns the position of the camera in world space
    Vec3<f32> position() const { return pivot.position + pivot.rotation * offset; }

    /// Returns the transformation from camera space to world space
    Rigid3<f32> transform() const { return {pivot.rotation, position()}; }
};

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