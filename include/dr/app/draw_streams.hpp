#pragma once

#include <dr/basic_types.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/hash.hpp>
#include <dr/hash_map.hpp>
#include <dr/sliced_array.hpp>
#include <dr/span.hpp>

#include <dr/app/gfx_resource.hpp>

namespace dr
{

struct StreamBuffer
{
    DynamicArray<u8> host_buffer;
    GfxBuffer device_buffer;
    usize host_offset{};
    usize device_size{};

    /// Appends a chunk of bytes to the host buffer, returning the offset to the beginning of the
    /// chunk. This corresponds with the offset of the data in the device buffer once transferred.
    i32 push(Span<u8 const> const& bytes);

    /// Transfers pushed data from host buffer to device buffer. This can be called repeatedly
    /// within the same frame to upload data in batches over multiple render passes. Returns false
    /// if the device buffer isn't large enough; it's automatically resized on the *first* transfer
    /// each frame to match the capacity of the host buffer.
    bool transfer(sg_buffer_usage usage);

    /// Clears the host buffer. Typically called once per frame, before pushing any data.
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

    bool transfer();

    void reset();

    GfxBuffer::Handle device_buffer() const { return buffer_.device_buffer; }

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

template <typename Index>
struct IndexStream
{
    i32 push(Span<Index const> const& indices);

    i32 push_once(void const* key, Span<Index const> const& indices);

    bool transfer();

    void reset();

    GfxBuffer::Handle device_buffer() const { return buffer_.device_buffer; }

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
