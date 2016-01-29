#pragma once

#include "base.hpp"
#include "game.hpp"

GGJ16_NAMESPACE
{
    enum class stat_type
    {
        health = 0,
        shield = 1,
        power = 2,
        defense = 3,
        favour = 4
    };

    constexpr sz_t stat_count{5};

    using stat_value = float;

    using stat = stat_value;

    using stat_array = std::array<stat, stat_count>;

    template <typename TArray>
    decltype(auto) get_stat(stat_type type, TArray && array) noexcept
    {
        return array[vrmc::from_enum(type)];
    }
}
GGJ16_NAMESPACE_END
