#pragma once

#include <dr/app/camera_rig.hpp>

namespace dr
{

struct OrbitCamera
{
    CameraRig rig;

    struct
    {
        Vec3<f32> position{};
        f32 radius{1.0f};
    } target;

    struct
    {
        f32 fov_y{deg_to_rad(60.0f)};
        f32 clip_near{0.01f};
        f32 clip_far{1000.0f};
    } frustum;

    struct
    {
        Orbit orbit{};
        Zoom zoom{{1.0f, 1.0f, 0.01, 1000.0}};
        Pan pan{};
        f32 stiffness{10.0f};
    } controls;

    struct
    {
        Vec2<f32> prev_touch_points[2];
        i8 prev_num_touches;
        bool mouse_down[3];
    } input;

    OrbitCamera();

    void update(f64 delta_time_sec);
    void frame_target();
    void frame_target_now();

    Mat4<f32> make_world_to_view() const;
    Mat4<f32> make_view_to_clip(f32 aspect) const;
};

} // namespace dr