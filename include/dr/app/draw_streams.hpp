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

struct BufferStage
{
    DynamicArray<u8> host_buf;
    GfxBuffer device_buf;
    usize device_size{};

    i32 append(Span<u8 const> const& bytes);
    void update_device(sg_buffer_usage usage);
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

    GfxBuffer::Handle device_buffer() const { return stage_.device_buf; }

    void update_device_buffer();

    void clear();

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

    BufferStage stage_;
    HashMap<Key, i32, Key::Hash> offsets_;

    i32 push_vertices_once(Key const& key, Span<u8 const> const& bytes);
};

template <typename Index>
struct IndexStream
{
    i32 push(Span<Index const> const& indices);

    i32 push_once(void const* key, Span<Index const> const& indices);

    GfxBuffer::Handle device_buffer() const { return stage_.device_buf; }

    void update_device_buffer();

    void clear();

  private:
    BufferStage stage_;
    HashMap<void const*, i32> offsets_;
};

struct UniformStream
{
    i32 push(Span<u8 const> const& bytes);

    i32 push_once(void const* key, Span<u8 const> const& bytes);

    Span<u8 const> operator[](i32 const slice) const { return stage_[slice]; }

    void clear();

  private:
    SlicedArray<u8> stage_;
    HashMap<void const*, i32> slices_;
};

} // namespace dr