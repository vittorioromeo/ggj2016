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
    class cenemy_state;

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

    enum class battle_event_type
    {
        player_damaged,
        enemy_damaged,
        player_healed,
        enemy_healed,
        player_shield_damaged,
        player_shield_healed,
        enemy_shield_damaged,
        enemy_shield_healed,
        enemy_stunned
    };

    namespace impl
    {
        struct b_damage_event
        {
            stat_value _amount;
        };

        struct b_heal_event
        {
            stat_value _amount;
        };

        struct b_stun_event
        {
            int _turns;
        };
    }

#define MAKE_ACCESSORS(name, to_ret)         \
    auto& name() noexcept { return to_ret; } \
    const auto& name() const noexcept { return to_ret; }

    class battle_event
    {
    private:
        battle_event_type _et;

        union
        {
            impl::b_damage_event _e_damage;
            impl::b_heal_event _e_heal;
            impl::b_stun_event _e_stun;
        } _u;

    public:
        battle_event(battle_event_type et) : _et(et) {}

        MAKE_ACCESSORS(e_damage, _u._e_damage)
        MAKE_ACCESSORS(e_heal, _u._e_heal)
        MAKE_ACCESSORS(e_stun, _u._e_stun)

        const auto& type() const noexcept { return _et; }
    };

#undef MAKE_ACCESSORS

    class battle_context_t
    {
    private:
        cplayer_state& _player_state;
        cenemy_state& _enemy_state;
        battle_t _battle;

        void notify_damage(battle_event_type et, stat_value x)
        {
            battle_event e{et};
            e.e_damage()._amount = x;
            on_event(e);
        }

        void notify_heal(battle_event_type et, stat_value x)
        {
            battle_event e{et};
            e.e_heal()._amount = x;
            on_event(e);
        }

        void notify_stun(battle_event_type et, int x)
        {
            battle_event e{et};
            e.e_stun()._turns = x;
            on_event(e);
        }


    public:
        ssvu::Delegate<void(battle_event)> on_event;

        battle_context_t(cplayer_state& player_state, cenemy_state& enemy_state,
            const battle_t& battle)
            : _player_state{player_state}, _enemy_state{enemy_state},
              _battle{battle}
        {
        }

        auto& player_state() noexcept { return _player_state; }
        const auto& player_state() const noexcept { return _player_state; }

        auto& enemy_state() noexcept { return _enemy_state; }
        const auto& enemy_state() const noexcept { return _enemy_state; }

        auto& battle() noexcept { return _battle; }
        const auto& battle() const noexcept { return _battle; }

        auto& player() noexcept { return battle().player(); }
        const auto& player() const noexcept { return battle().player(); }

        auto& enemy() noexcept { return battle().enemy(); }
        const auto& enemy() const noexcept { return battle().enemy(); }

        template <typename T>
        auto damage_calc(T& stats, stat_value x)
        {
            auto& h(stats.health());
            auto& s(stats.shield());
            auto& ms(stats.maxshield());

            auto sratio(s / ms);
            auto blocked_dmg(x * (sratio * 0.9f));
            auto dmg(x - blocked_dmg);


            h -= dmg;
            ssvu::clampMin(h, 0);

            return dmg;
        }

        void damage_player_by(stat_value x)
        {
            x = damage_calc(player().stats(), x);
            notify_damage(battle_event_type::player_damaged, x);
        }

        void damage_enemy_by(stat_value x)
        {
            x = damage_calc(enemy().stats(), x);
            notify_damage(battle_event_type::enemy_damaged, x);
        }

        void heal_player_by(stat_value x)
        {
            auto& s(player().stats().health());
            s += x;
            ssvu::clampMax(s, player().stats().maxhealth());

            notify_heal(battle_event_type::player_healed, x);
        }

        void heal_enemy_by(stat_value x)
        {
            auto& s(enemy().stats().health());
            s += x;
            ssvu::clampMax(s, enemy().stats().maxhealth());

            notify_heal(battle_event_type::enemy_healed, x);
        }

        void damage_player_shield_by(stat_value x)
        {
            auto& s(player().stats().shield());
            s -= x;
            ssvu::clampMin(s, 0);

            notify_damage(battle_event_type::player_shield_damaged, x);
        }

        void damage_enemy_shield_by(stat_value x)
        {
            auto& s(enemy().stats().shield());
            s -= x;
            ssvu::clampMin(s, 0);

            notify_damage(battle_event_type::enemy_shield_damaged, x);
        }

        void heal_player_shield_by(stat_value x)
        {
            auto& s(player().stats().shield());
            s += x;
            ssvu::clampMax(s, player().stats().maxshield());

            notify_heal(battle_event_type::player_shield_healed, x);
        }

        void heal_enemy_shield_by(stat_value x)
        {
            auto& s(enemy().stats().shield());
            s += x;
            ssvu::clampMax(s, enemy().stats().maxshield());

            notify_heal(battle_event_type::enemy_shield_healed, x);
        }

        void stun_enemy_for(int x)
        {
            enemy().stunned_for() = x;
            notify_stun(battle_event_type::enemy_stunned, x);
        }
    };
}
GGJ16_NAMESPACE_END
