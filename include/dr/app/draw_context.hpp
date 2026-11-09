#pragma once

#include <sokol_gfx.h>

#include <dr/constants.hpp>
#include <dr/container_utils.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/num_types.hpp>
#include <dr/span.hpp>

#include <dr/app/draw_command.hpp>
#include <dr/app/draw_streams.hpp>

namespace dr
{

struct DrawContext
{
    VertexStream vertex_stream;
    IndexStream index_stream;
    UniformStream uniform_stream;

    /// Call once per frame before adding any draw commands. Invalidates all batch handles.
    void next_frame();

    struct BatchHandle
    {
        i32 index{invalid_index<i32>};

        constexpr bool is_valid() const { return index != invalid_index<i32>; }
        constexpr explicit operator bool() const { return is_valid(); }
    };

    /// Opens a new batch of draw commands
    void begin_batch(Span<u8 const> const& uniform_data);

    /// Records a draw command to the current batch
    void draw(DrawCommand const& cmd);

    /// Closes the current batch of draw commands
    BatchHandle end_batch();

    /// Repeats a previously recorded batch of draw commands
    BatchHandle repeat_batch(BatchHandle handle, Span<u8 const> const& uniform_data);

    /// Orders draw commands within each batch and transfers streamed data to device buffers
    void commit_batches();

    /// Submits a batch of draw commands
    void submit_batch(BatchHandle handle, sg_bindings const& base_bindings);

    /// Returns the number of batches created so far this frame
    isize num_batches() const { return size(batches_); }

  private:
    struct Batch
    {
        i32 uniform_slice{invalid_index<i32>};
        i32 cmd_from{invalid_index<i32>};
        i32 cmd_to{invalid_index<i32>};
        bool is_repeat{};
    };

    enum Stage : u8
    {
        Undefined = 0,
        Record,
        Submit,
        // ...
    };

    DynamicArray<DrawCommand> draw_cmds_;
    DynamicArray<Batch> batches_;
    Stage stage_{};

    /// Returns a pointer to the currently open batch
    Batch* curr_batch();
};

} // namespace dr
