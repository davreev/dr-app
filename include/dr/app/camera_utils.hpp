#pragma once

#include <dr/app/camera.hpp>

namespace dr
{

/// Creates a camera from the given controls
Camera make_camera(Orbit const& orbit, Zoom const& zoom);

/// Creates a camera looking down the x axis at a given target
Camera make_camera_front(Vec3<f32> const& target, f32 offset);

/// Creates a camera looking up the x axis at a given target
Camera make_camera_back(Vec3<f32> const& target, f32 offset);

/// Creates a camera located at a given position looking at a given target
Camera make_camera_look_at(Vec3<f32> const& position, Vec3<f32> const& target, Vec3<f32> const& up);

/// Translates the camera to fit a given sphere in view
void frame_bounds(Camera& camera, Vec3<f32> const& center, f32 radius, f32 fov_min);

} // namespace dr