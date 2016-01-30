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
            CACHE_ASSETS(sf::Texture, ".png", landscape, d0, d1, d2, d3)

            // Sounds
            CACHE_ASSETS(sf::SoundBuffer, ".ogg", click0, enemy_atk0,
                enemy_atk1, msgbox, shield_up, fireball, obliterate, success,
                failure, scripted_text, blip, bip)

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

            template <typename T>
            inline void psnd(T&& s, ssvs::SoundPlayer::Mode mMode =
                                        ssvs::SoundPlayer::Mode::Overlap,
                float mPitch = 1.f)
            {
                _sound_player.play(*s, mMode, mPitch);
            }

            template <typename... Ts>
            inline void psnd_one(Ts... xs)
            {
                std::vector<std::common_type_t<Ts...>> v{xs...};
                ssvu::shuffle(v);
                psnd(v[0]);
            }
        };
    }

    inline auto& assets() noexcept
    {
        static impl::assets result;
        return result;
    }

    inline auto mkTxtOBSmall()
    {
        ssvs::BitmapText result{*assets().fontObStroked};
        result.setTracking(-3);
        return result;
    }
    inline auto mkTxtOBBig()
    {
        ssvs::BitmapText result{*assets().fontObBig};
        result.setTracking(-1);
        return result;
    }
}
GGJ16_NAMESPACE_END
