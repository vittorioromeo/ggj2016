#pragma once

#include "base.hpp"
#include "game.hpp"

#include "battle/stat.hpp"
#include "battle/character_stats.hpp"
#include "battle/stat_effect.hpp"
#include "battle/targeted_stat_effect.hpp"
#include "battle/stat_buff.hpp"

GGJ16_NAMESPACE
{
    class battle_participant
    {
    private:
        character_stats _stats;
        int _priority{0};
        int _stunned_for{0};

    public:
        battle_participant(const character_stats& stats) : _stats{stats} {}

        auto& stats() noexcept { return _stats; }
        const auto& stats() const noexcept { return _stats; }

        auto& stunned_for() noexcept { return _stunned_for; }
        const auto& stunned_for() const noexcept { return _stunned_for; }
    };

    /// @brief What happens to a battle participant after the execution of a
    /// ritual.
    class ritual_effect
    {
    private:
        std::function<void(battle_participant&)> _effect_fn;

    public:
        template <typename TF>
        ritual_effect(TF&& effect_fn)
            : _effect_fn(effect_fn)
        {
        }

        void apply_to_participant(battle_participant& p) { _effect_fn(p); }
    };
}
GGJ16_NAMESPACE_END
