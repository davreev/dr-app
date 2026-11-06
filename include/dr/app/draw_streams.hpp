#pragma once

#include <sokol_gfx.h>

#include <dr/dynamic_array.hpp>
#include <dr/hash.hpp>
#include <dr/hash_map.hpp>
#include <dr/num_types.hpp>
#include <dr/sliced_array.hpp>
#include <dr/span.hpp>

#include <dr/app/gfx_resource.hpp>

namespace dr
{

struct StreamBuffer
{
    DynamicArray<u8> host_buffer;
    GfxBuffer device_buffer;
    usize device_size{};

    /// Appends a chunk of bytes to the host buffer, returning the offset to the beginning of the
    /// appended chunk.
    i32 push(Span<u8 const> const& bytes, usize align);

    /// Transfers pushed data from host buffer to device buffer. Call once per frame after all data
    /// is pushed.
    void transfer(sg_buffer_usage usage);

    /// Clears the host buffer. Call once per frame, before pushing any data.
    void reset();
};

struct VertexStream
{
    static constexpr u8 num_slots = SG_MAX_VERTEXBUFFER_BINDSLOTS;

    i32 push(Span<u8 const> const& bytes);

    template <u8 slot>
    i32 push_once(void const* key, Span<u8 const> const& bytes)
    {
        static_assert(slot < num_slots);
        return push_vertices_once({key, slot}, bytes);
    }

    void transfer();

    void reset();

    GfxBuffer::Handle device_buffer();

  private:
    struct Key
    {
        void const* src;
        u8 slot;
        bool operator==(Key const& other) const;

        struct Hash : HighQualityHash
        {
            usize operator()(Key const& key) const;
        };
    };

    StreamBuffer buffer_;
    HashMap<Key, i32, Key::Hash> offsets_;

    i32 push_vertices_once(Key const& key, Span<u8 const> const& bytes);
};

struct IndexStream
{
    i32 push(Span<u8 const> const& bytes);

    i32 push_once(void const* key, Span<u8 const> const& bytes);

    void transfer();

    void reset();

    GfxBuffer::Handle device_buffer();

  private:
    StreamBuffer buffer_;
    HashMap<void const*, i32> offsets_;
};

struct UniformStream
{
    i32 push(Span<u8 const> const& bytes);

    i32 push_once(void const* key, Span<u8 const> const& bytes);

    void reset();

    Span<u8 const> operator[](i32 const slice) const { return buffer_[slice]; }

  private:
    SlicedArray<u8> buffer_;
    HashMap<void const*, i32> slices_;
};

} // namespace dr
