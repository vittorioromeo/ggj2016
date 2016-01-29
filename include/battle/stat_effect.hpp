#pragma once

#include "base.hpp"
#include "game.hpp"

#include "battle/stat.hpp"
#include "battle/character_stats.hpp"

GGJ16_NAMESPACE
{
    using stat_effect_fn = std::function<stat(stat)>;

    class stat_effect
    {
    private:
        stat_effect_fn _effect_fn;

    public:
        stat_effect(const stat_effect_fn& effect_fn) : _effect_fn{effect_fn} {}
        auto compute(stat s) const { return _effect_fn(s); }
    };

    /// @brief Applies an effect to a stat, preserving the original stat and
    /// returning a modified copy.
    auto apply_stat_effect(const stat_effect& e, const stat& s)
    {
        return e.compute(s);
    }
}
GGJ16_NAMESPACE_END
