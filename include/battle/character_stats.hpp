#pragma once

#include "base.hpp"
#include "game.hpp"

#include "battle/stat.hpp"

#define GGJ16_DEFINE_STAT_ACCESSOR(name, type)  \
    auto& name() noexcept { return get(type); } \
    const auto& name() const noexcept { return get(type); }

GGJ16_NAMESPACE
{
    class character_stats
    {
    private:
        stat_array _stat_array;

        auto& get(stat_type type) noexcept
        {
            return get_stat(type, _stat_array);
        }

        const auto& get(stat_type type) const noexcept
        {
            return get_stat(type, _stat_array);
        }

    public:
        character_stats() = default;
        character_stats(const character_stats&) = default;
        character_stats& operator=(const character_stats&) = default;

        GGJ16_DEFINE_STAT_ACCESSOR(health, stat_type::health)
        GGJ16_DEFINE_STAT_ACCESSOR(shield, stat_type::shield)
        GGJ16_DEFINE_STAT_ACCESSOR(power, stat_type::power)
        GGJ16_DEFINE_STAT_ACCESSOR(maxhealth, stat_type::maxhealth)
        GGJ16_DEFINE_STAT_ACCESSOR(maxshield, stat_type::maxshield)
    };
}
GGJ16_NAMESPACE_END
