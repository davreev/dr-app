#pragma once

#include <dr/basic_types.hpp>
#include <dr/dynamic_array.hpp>
#include <dr/hash.hpp>
#include <dr/hash_map.hpp>
#include <dr/span.hpp>

#include <dr/app/gfx_resource.hpp>

namespace dr
{

struct GeometryStream
{
    static constexpr u8 num_vertex_slots = SG_MAX_VERTEXBUFFER_BINDSLOTS;

    i32 push_vertices(Span<u8 const> const& data);

    template <u8 slot>
    i32 push_vertices_once(void const* key, Span<u8 const> const& data)
    {
        static_assert(slot < num_vertex_slots);
        return push_vertices_once({key, slot}, data);
    }

    i32 push_indices(Span<u8 const> const& data);

    i32 push_indices_once(void const* key, Span<u8 const> const& data);

    GfxBuffer::Handle vertex_buffer() const { return vertex_stage_.device_buf; }

    GfxBuffer::Handle index_buffer() const { return index_stage_.device_buf; }

    void update_device_buffers();

    void clear();

  private:
    struct BufferStage
    {
        DynamicArray<u8> host_buf;
        GfxBuffer device_buf;
        usize device_size;

        i32 append(Span<u8 const> const& bytes);
        void update_device(sg_buffer_usage usage);
    };

    struct VertexKey
    {
        void const* src;
        u8 slot;
        bool operator==(VertexKey const& other) const;

        struct Hash : HighQualityHash
        {
            usize operator()(VertexKey const& key) const;
        };
    };

    BufferStage vertex_stage_;
    BufferStage index_stage_;
    HashMap<VertexKey, i32, VertexKey::Hash> vertex_offsets_;
    HashMap<void const*, i32> index_offsets_;

    i32 push_vertices_once(VertexKey const& key, Span<u8 const> const& data);
};

} // namespace dr