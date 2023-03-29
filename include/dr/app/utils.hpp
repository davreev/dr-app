#pragma once

#include <cmath>

#include <fstream>
#include <vector>

#include <sokol_app.h>

#include <dr/math_types.hpp>
#include <dr/span.hpp>

#include "app.hpp"
#include "camera.hpp"

namespace dr
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// I/O

/// Reads a file to a given buffer
bool read_text_file(char const* path, std::string& buffer);

/// Reads a file to a given buffer
bool read_binary_file(char const* path, std::vector<u8>& buffer);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Input

/// True if the mouse is over the app window
bool is_mouse_over(sapp_event const* event);

/// Scale factor for taking input deltas from screen to view space (at z = 1).
f32 screen_to_view_scale(f32 fov, f32 size);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Camera

/// Creates a camera from the given controls
Camera make_camera(Orbit const& orbit, Zoom const& zoom);

/// Creates a camera looking down the x axis at a given target
Camera camera_front(Vec3<f32> const& target, f32 offset);

/// Creates a camera looking up the x axis at a given target
Camera camera_back(Vec3<f32> const& target, f32 offset);

/// Creates a camera located at a given position looking at a given target
Camera camera_look_at(Vec3<f32> const& position, Vec3<f32> const& target, Vec3<f32> const& up);

/// Translates the camera to fit a given sphere in view
void zoom_to_fit(Camera& camera, Vec3<f32> const& center, f32 radius, f32 fov_min);

/// Returns zoom distance required to fit a given sphere in view
f32 zoom_distance(f32 radius, f32 fov_min);

/// Handles mouse events for camera control
void camera_handle_mouse(
    Camera& camera,
    Orbit* orbit,
    Zoom* zoom,
    Pan* pan,
    bool* mouse_down,
    f32 move_scale,
    sapp_event const& event);

/// Handles touch events for camera control
void camera_handle_touch(
    Camera& camera,
    Orbit* orbit,
    Zoom* zoom,
    Pan* pan,
    Vec2<f32>* last_touch_points,
    i8& last_num_touches,
    f32 move_scale,
    sapp_event const& event);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Transforms

Mat4<f32> scale(Vec3<f32> const& scale);

Mat4<f32> translate(Vec3<f32> const& translate);

Mat4<f32> scale_translate(Vec3<f32> const& scale, Vec3<f32> const& translate);

Mat3<f32> ortho_basis(Vec3<f32> const& unit_x, Vec3<f32> const& xy);

Mat4<f32> as_affine(Mat3<f32> const& linear);

template <bool use_ogl_ndc = false>
Mat4<f32> perspective(f32 fov_y, f32 aspect, f32 near, f32 far);

Mat4<f32> look_at(Vec3<f32> const& eye, Vec3<f32> const& target, Vec3<f32> const& up);

Mat2<f32> rotate(f32 angle);

Vec4<f32> to_plane_eqn(Vec3<f32> const& origin, Vec3<f32> const& normal);

} // namespace dr