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
    class cplayer_state;

    class battle_t
    {
    private:
        battle_participant _player;
        battle_participant _enemy;
        bool _player_plays{true};

    public:
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


    public:
        battle_t(
            const battle_participant& player, const battle_participant& enemy)
            : _player{player}, _enemy{enemy}
        {
        }

        auto player_dead() { return _player.stats().health() < 0; }
        auto enemy_dead() { return _enemy.stats().health() < 0; }
        auto must_continue() { return !player_dead() && !enemy_dead(); }
        auto is_player_turn() { return _player_plays; }

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

    class battle_context_t
    {
    private:
        cplayer_state& _player_state;
        battle_t _battle;

    public:
        battle_context_t(cplayer_state& player_state, const battle_t& battle)
            : _player_state{player_state}, _battle{battle}
        {
        }

        auto& player_state() noexcept { return _player_state; }
        const auto& player_state() const noexcept { return _player_state; }

        auto& battle() noexcept { return _battle; }
        const auto& battle() const noexcept { return _battle; }
    };
}
GGJ16_NAMESPACE_END
