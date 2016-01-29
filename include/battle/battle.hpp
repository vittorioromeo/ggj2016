#pragma once

#include "base.hpp"
#include "game.hpp"

#include "battle/stat.hpp"
#include "battle/character_stats.hpp"
#include "battle/stat_effect.hpp"
#include "battle/targeted_stat_effect.hpp"
#include "battle/stat_buff.hpp"
#include "battle/battle_participant.hpp"

GGJ16_NAMESPACE
{
    class cplayer_state
    {
    private:
        // vector of available items
        // vector of available rituals

    public:
    };

    class battle
    {
    private:
        cplayer_state& _player_state;
        battle_participant _player;
        battle_participant _enemy;
        bool _player_plays{true};

    public:
        auto& player_state() noexcept { return _player_state; }
        const auto& player_state() const noexcept { return _player_state; }

        auto& player() noexcept { return _player; }
        const auto& player() const noexcept { return _player; }

        auto& enemy() noexcept { return _enemy; }
        const auto& enemy() const noexcept { return _enemy; }

    private:
        void player_turn()
        {
            // Wait for ritual/action selection
            // Execute minigame
            // Apply effects on enem
        }

        void enemy_turn()
        {
            // Make AI decision
            // Apply effect on player
        }

        void next_turn()
        {
            if(_player_plays)
            {
                player_turn();
            }
            else
            {
                enemy_turn();
            }

            _player_plays = !_player_plays;
        }

        auto player_dead() { return _player.stats().health() < 0; }
        auto enemy_dead() { return _enemy.stats().health() < 0; }
        auto must_continue() { return !player_dead() && !enemy_dead(); }

    public:
        battle(cplayer_state& player_state, const battle_participant& player,
            const battle_participant& enemy)
            : _player_state{player_state}, _player{player}, _enemy{enemy}
        {
        }

        void execute_battle()
        {
            while(must_continue())
            {
                next_turn();
            }

            if(player_dead())
            {
                // Game over.
            }

            if(enemy_dead())
            {
            }
        }
    };
}
GGJ16_NAMESPACE_END
