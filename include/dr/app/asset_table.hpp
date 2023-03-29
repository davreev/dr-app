#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace dr
{

template <typename Data, typename Handle, int size>
struct AssetTable
{
    void set_path(Handle const id, const std::string_view path)
    {
        assets_[id].path = path;
    }

    const Data* get(Handle const id, bool const force_reload = false)
    {
        auto& asset = assets_[id];

        if(asset.data == nullptr || force_reload)
            load(asset);

        return asset.data.get();
    }

    void release(Handle const id)
    {
        assets_[id].data = nullptr;
    }

    bool load_all(bool const force_reload = false)
    {
        bool success = true;

        for(auto& asset : assets_)
        {
            if(asset.data == nullptr || force_reload)
                load(asset);

            success &= (asset.data != nullptr);
        }

        return success;
    }

    void release_all()
    {
        for(auto& asset : assets_)
            asset.data = nullptr;
    }

  private:
    struct Asset
    {
        std::string path{};
        std::unique_ptr<Data> data{};
    };

    Asset assets_[size];

    // Specialize this function for different asset data types
    void load(Asset& asset);
};

} // namespace dr