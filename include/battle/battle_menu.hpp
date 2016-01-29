#pragma once

#include "base.hpp"
#include "game.hpp"
#include "assets.hpp"

#include "battle/stat.hpp"
#include "battle/character_stats.hpp"
#include "battle/stat_effect.hpp"
#include "battle/targeted_stat_effect.hpp"
#include "battle/stat_buff.hpp"
#include "battle/battle_participant.hpp"
#include "battle/battle.hpp"

GGJ16_NAMESPACE
{
    class battle_menu;

    using battle_menu_choice_fn = std::function<void(battle_menu&)>;

    class battle_menu_choice
    {
    private:
        std::string _label;
        battle_menu_choice_fn _fn;

    public:
        template <typename TF>
        battle_menu_choice(const std::string& label, TF&& fn)
            : _label{label}, _fn{fn}
        {
        }

        void execute(battle_menu& bm) const { _fn(bm); }
        const auto& label() const noexcept { return _label; }
    };

    using choice_set = std::vector<std::unique_ptr<battle_menu_choice>>;

    class battle_menu_screen
    {
    private:
        choice_set _choices;

    public:
        template <typename... Ts>
        auto& emplace_choice(Ts&&... xs)
        {
            _choices.emplace_back(
                std::make_unique<battle_menu_choice>(FWD(xs)...));

            return *(_choices.back());
        }

        template <typename TF>
        void for_choices(TF&& f) const
        {
            for(const auto& c : _choices) f(*c);
        }

        void clear() { _choices.clear(); }
    };

    class battle_menu
    {
    private:
        std::vector<std::unique_ptr<battle_menu_screen>> _screens;
        std::vector<battle_menu_screen*> _screen_stack;

    public:
        ssvu::Delegate<void()> on_change;

        battle_menu()
        {
            _screens.reserve(10);
            _screen_stack.reserve(10);
        }

        auto& current_screen() noexcept
        {
            VRM_CORE_ASSERT(!_screen_stack.empty());
            return *(_screen_stack.back());
        }

        const auto& current_screen() const noexcept
        {
            VRM_CORE_ASSERT(!_screen_stack.empty());
            return *(_screen_stack.back());
        }

        void push_screen(battle_menu_screen& s)
        {
            _screen_stack.emplace_back(&s);
            on_change();
        }

        void pop_screen()
        {
            VRM_CORE_ASSERT(!_screen_stack.empty());
            _screen_stack.pop_back();
            on_change();
        }

        template <typename... Ts>
        auto& make_screen(Ts&&... xs)
        {
            auto uptr(std::make_unique<battle_menu_screen>(FWD(xs)...));
            _screens.emplace_back(std::move(uptr));
            return *(_screens.back());
        }
    };

    class battle_menu_gfx_button
    {
    private:
        sf::RectangleShape _shape;
        ssvs::BitmapTextRich _tr{*assets().fontObStroked};
        const battle_menu_choice& _bmc;
        vec2f _pos;
        bool _was_pressed{false};

        auto is_hovered(game_app& app)
        {
            auto mp(app.window().getMousePosition());

            auto l(ssvs::getGlobalLeft(_shape));
            auto r(ssvs::getGlobalRight(_shape));
            auto t(ssvs::getGlobalTop(_shape));
            auto b(ssvs::getGlobalBottom(_shape));

            if(mp.x > l && mp.x < r && mp.y > t && mp.y < b) return true;
            return false;
        }

    public:
        battle_menu_gfx_button(const vec2f& pos, const battle_menu_choice& bmc)
            : _pos(pos), _bmc(bmc)
        {
            _tr.setAlign(ssvs::TextAlign::Left);
            _tr.in(_bmc.label()).mk("");

            vec2f shape_sz(130, 20);
            _shape.setSize(shape_sz);
            _shape.setOrigin(shape_sz / 2.f);

            _tr.setOrigin(shape_sz / 2.f);
        }

        void update(game_app& app, battle_menu& bm, ft dt)
        {
            _shape.setPosition(_pos);
            _shape.setFillColor(is_hovered(app) ? sfc::Green : sfc::Red);

            auto lbtn_down(app.window().getInputState()[ssvs::MBtn::Left]);

            if(is_hovered(app) && !_was_pressed && lbtn_down)
            {
                _was_pressed = true;
                _bmc.execute(bm);
            }

            if(!lbtn_down)
            {
                _was_pressed = false;
            }

            _tr.setPosition(_pos);

            _tr.update(dt);
        }

        void draw(sf::RenderTarget& rt)
        {
            rt.draw(_shape);
            rt.draw(_tr);
        }
    };

    class battle_menu_gfx_state
    {
    private:
        std::vector<battle_menu_gfx_button> _buttons;

        void rebuild_from(const battle_menu_screen& bs)
        {
            _buttons.clear();

            vec2f start_pos(100, 60);
            int i = 0;
            bs.for_choices([this, &i, &start_pos](const auto& c)
                {
                    auto pos(start_pos + vec2f(0, 40 * i));
                    _buttons.emplace_back(pos, c);
                    ++i;
                });
        }

    public:
        battle_menu_gfx_state() { _buttons.reserve(10); }

        void rebuild_from(const battle_menu& bm)
        {
            rebuild_from(bm.current_screen());
        }

        void update(game_app& app, battle_menu& bm, ft dt)
        {
            for(auto& b : _buttons) b.update(app, bm, dt);
        }

        void draw(sf::RenderTarget& rt)
        {
            for(auto& b : _buttons) b.draw(rt);
        }
    };
}
GGJ16_NAMESPACE_END
