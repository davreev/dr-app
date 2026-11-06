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

    struct BatchInfo
    {
        sg_bindings bindings;
        Span<u8 const> uniform_data;
    };

    struct BatchHandle
    {
        i32 index{invalid_index<i32>};

        constexpr bool is_valid() const { return index != invalid_index<i32>; }
        constexpr explicit operator bool() const { return is_valid(); }
    };

    /// Opens a new batch of draw commands
    void begin_batch(BatchInfo const& info);

    /// Records a draw command to the current batch
    void draw(DrawCommand const& cmd);

    /// Closes the current batch of draw commands
    BatchHandle end_batch();

    /// Repeats an existing batch of draw commands using new info
    BatchHandle repeat_batch(BatchHandle handle, BatchInfo const& info);

    /// Orders draw commands within each batch and transfers streamed data to device buffers
    void commit_batches();

    /// Submits a batch of draw commands
    void submit_batch(BatchHandle handle);

    /// Returns the number of batches created so far this frame
    isize num_batches() const { return size(batches_); }

  private:
    struct Batch
    {
        sg_bindings bindings;
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
