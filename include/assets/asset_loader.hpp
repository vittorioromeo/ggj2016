#pragma once

#include "base.hpp"

GGJ16_NAMESPACE
{
    struct asset_loader
    {
        ssvs::AssetManager<> _asset_manager;

        inline asset_loader()
        {
            ssvs::loadAssetsFromJson(
                _asset_manager, "data/", ssvj::fromFile("data/assets.json"));
        }
    };
}
GGJ16_NAMESPACE_END
