#pragma once

#include "base.hpp"
#include "assets/asset_loader.hpp"


#define CACHE_ASSET_IMPL(mType, mName, mExt) \
    mType* mName{                            \
        &_asset_loader._asset_manager.get<mType>(VRM_PP_TOSTR(mName) mExt)};

#define CACHE_ASSETS_FOR_IMPL(mIdx, mData, mArg) \
    CACHE_ASSET_IMPL(VRM_PP_TPL_ELEM(mData, 0), mArg, VRM_PP_TPL_ELEM(mData, 1))

#define CACHE_ASSETS(mType, mExt, ...) \
    VRM_PP_FOREACH_REVERSE(            \
        CACHE_ASSETS_FOR_IMPL, VRM_PP_TPL_MAKE(mType, mExt), __VA_ARGS__)

GGJ16_NAMESPACE
{
    namespace impl
    {
        struct assets
        {
            asset_loader _asset_loader;

            // Audio players
            ssvs::SoundPlayer _sound_player;
            ssvs::MusicPlayer _music_player;

            // BitmapFonts
            CACHE_ASSETS(ssvs::BitmapFont, "", fontObStroked, fontObBig)

            // Textures
            /*
    CACHE_ASSETS(sf::Texture, ".png", slotChoice, slotChoiceBlocked,
        iconHPS, iconATK, iconDEF, drops, enemy, blocked, back, dropsModal,
        advance, itemCard, eFire, eWater, eEarth, eLightning, eST, eWK, eTY,
        equipCard, wpnMace, wpnSword, wpnSpear, armDrop, panelsmall,
        panellog)
                */

            // Sounds
            /*
    CACHE_ASSETS(sf::SoundBuffer, ".ogg", lvl1, lvl2, lvl3, lvl4, menu,
        powerup, drop, grab, equipArmor, equipWpn, lose)
                */

            /*
    std::vector<sf::SoundBuffer *> swordSnds, maceSnds, spearSnds;
    */
            inline assets()
            {
                /*
                for(const auto& e :
                    {"normal"s, "fire"s, "water"s, "earth"s, "lightning"s})
                {
                    swordSnds.emplace_back(
                        &assetLoader.assetManager.get<sf::SoundBuffer>(
                            "sword/" + e + ".ogg"));
                    maceSnds.emplace_back(
                        &assetLoader.assetManager.get<sf::SoundBuffer>(
                            "mace/" + e + ".ogg"));
                    spearSnds.emplace_back(
                        &assetLoader.assetManager.get<sf::SoundBuffer>(
                            "spear/" + e + ".ogg"));
                }
                */

                _sound_player.setVolume(100.f);
            }
        };
    }

    inline auto& assets() noexcept
    {
        static impl::assets result;
        return result;
    }
}
GGJ16_NAMESPACE_END
