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

    i32 push_vertices(Span<u8 const> const& data);

    i32 push_vertices_once(VertexKey const& key, Span<u8 const> const&);

    i32 push_indices(Span<u8 const> const& data);

    i32 push_indices_once(void const* key, Span<u8 const> const& data);

    GfxBuffer::Handle vertex_buffer() const { return vertex_.device.buffer; }

    GfxBuffer::Handle index_buffer() const { return index_.device.buffer; }

    void update_device_buffers();

    void clear();

  private:
    struct DeviceBuffer
    {
        GfxBuffer buffer;
        usize capacity;

        void init(GfxBuffer::Desc const& desc);
    };

    struct VertexStage
    {
        DynamicArray<u8> host;
        DeviceBuffer device;
        HashMap<VertexKey, i32, VertexKey::Hash> offsets;

        void update_device();
    };

    struct IndexStage
    {
        DynamicArray<u8> host;
        DeviceBuffer device;
        HashMap<void const*, i32> offsets;
        
        void update_device();
    };

    VertexStage vertex_;
    IndexStage index_;
};

} // namespace dr