#pragma once

#include <dr/app/camera_controls.hpp>
#include <dr/app/gfx_utils.hpp>

namespace dr
{

struct OrbitCamera
{
    enum Projection : u8
    {
        Projection_Perspective = 0,
        Projection_Orthographic,
    };

    CameraRig rig;

    struct
    {
        Vec3<f32> position;
        f32 radius{1.0f};
    } target;

    struct
    {
        f32 fov_y{pi<f32> / 3.0f};
        f32 clip_near{0.01f};
        f32 clip_far{1.0e5f};
    } frustum;

    struct
    {
        Orbit orbit;
        Zoom zoom{.distance{.current = 1.0f, .min = 0.01f, .max = 1000.0f,}};
        Pan pan;
        f32 stiffness{10.0f};
    } controls;

    struct
    {
        Vec2<f32> prev_touch_points[2];
        i8 prev_num_touches{};
        bool mouse_down[3]{};
    } input;

    Projection projection{};

    OrbitCamera();

    OrbitCamera(CameraRig const& rig);

    void set_rig(CameraRig const& value);

    void set_rig_now(CameraRig const& value);

    void frame_target();

    void frame_target_now();

    void update(f64 delta_time_sec);

    Mat4<f32> make_world_to_view() const;

    template <NdcType ndc = NdcType_Default>
    Mat4<f32> make_view_to_clip(f32 aspect) const;
};

} // namespace dr