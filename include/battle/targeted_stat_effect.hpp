#pragma once

#include "base.hpp"
#include "game.hpp"

#include "battle/stat.hpp"
#include "battle/character_stats.hpp"
#include "battle/stat_effect.hpp"

GGJ16_NAMESPACE
{
    class targeted_stat_effect
    {
    private:
        stat_type _target_type;
        stat_effect _effect;

    public:
        targeted_stat_effect(stat_type target_type, const stat_effect& effect)
            : _target_type{target_type}, _effect{effect}
        {
        }

        void compute(stat_array& array) const
        {
            const auto& s(get_stat(_target_type, array));

            array[vrmc::from_enum(_target_type)] =
                apply_stat_effect(_effect, s);
        }
    };

    /// @brief Applies a targeteted stat effect mutating the array `a`.
    auto apply_targeted_stat_effect(
        const targeted_stat_effect& e, stat_array& a)
    {
        return e.compute(a);
    }
}
GGJ16_NAMESPACE_END
