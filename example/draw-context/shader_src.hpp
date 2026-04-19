#pragma once

namespace dr
{

#ifdef __EMSCRIPTEN__
#define VERTEX_HEADER "#version 300 es"
#define FRAGMENT_HEADER "#version 300 es\nprecision mediump float;"
#else
#define VERTEX_HEADER "#version 330 core"
#define FRAGMENT_HEADER "#version 330 core"
#endif

constexpr char vertex_shader_src[] = VERTEX_HEADER R"(

uniform mat4 u_view_to_clip;
uniform mat4 u_local_to_view;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_instance_position;

out vec3 v_color;

void main()
{
    vec3 local_pos = a_position + a_instance_position;
    gl_Position = u_view_to_clip * u_local_to_view * vec4(local_pos, 1.0);
    v_color = a_color;
}

)";

constexpr char fragment_shader_src[] = FRAGMENT_HEADER R"(

in vec3 v_color;

out vec4 f_color;

void main()
{
    f_color = vec4(v_color, 1.0);
}

)";

} // namespace dr
