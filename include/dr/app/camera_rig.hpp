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

    /// Translates the camera rig to fit a given bounding sphere in view
    void frame_bounds(CameraRig& rig, Vec3<f32> const& center, f32 radius, f32 fov_min);

    /// Transitions the camera rig to another
    void transition_to(CameraRig const& other, f32 const t);

    /// Creates a camera rig looking at the xz plane with the x axis pointing to the right and the z
    /// axis pointing up
    static CameraRig make_front(Vec3<f32> const& origin, f32 offset);

    /// Creates a camera rig looking at the xz plane with the x axis pointing to the left and the z
    /// axis pointing up
    static CameraRig make_back(Vec3<f32> const& origin, f32 offset);

    /// Creates a camera rig looking at the yz plane with the y axis pointing to the left and the z
    /// axis pointing up
    static CameraRig make_right(Vec3<f32> const& origin, f32 offset);

    /// Creates a camera rig looking at the yz plane with the y axis pointing to the right and the z
    /// axis pointing up
    static CameraRig make_left(Vec3<f32> const& origin, f32 offset);

    /// Creates a camera rig looking at the xy plane with the x axis pointing to the right and the y
    /// axis pointing up
    static CameraRig make_top(Vec3<f32> const& origin, f32 offset);

    /// Creates a camera rig looking at the xy plane with the x axis pointing to the left and the y
    /// axis pointing up
    static CameraRig make_bottom(Vec3<f32> const& origin, f32 offset);

    /// Creates a camera rig located at a given position looking at a given target
    static CameraRig make_look_at(
        Vec3<f32> const& position,
        Vec3<f32> const& target,
        Vec3<f32> const& up);
};

} // namespace dr