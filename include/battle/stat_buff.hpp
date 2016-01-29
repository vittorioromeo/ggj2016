#pragma once

#include "base.hpp"
#include "game.hpp"

#include "battle/stat.hpp"
#include "battle/character_stats.hpp"
#include "battle/stat_effect.hpp"
#include "battle/targeted_stat_effect.hpp"

GGJ16_NAMESPACE
{
    class stat_buff
    {
    private:
        std::vector<targeted_stat_effect> _effects;

    public:
        auto compute(stat_array array)
        {
            for(const auto& e : _effects)
            {
                apply_targeted_stat_effect(e, array);
            }

            return array;
        }
    };
}
GGJ16_NAMESPACE_END
