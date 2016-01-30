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
        bool _was_pressed{false};
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
        vec2f _pos;
        const battle_menu_choice& _bmc;

        auto is_hovered(game_app& app)
        {
            auto mp(app.mp());

            auto l(ssvs::getGlobalLeft(_shape));
            auto r(ssvs::getGlobalRight(_shape));
            auto t(ssvs::getGlobalTop(_shape));
            auto b(ssvs::getGlobalBottom(_shape));

            if(mp.x > l && mp.x < r && mp.y > t && mp.y < b) return true;
            return false;
        }


    public:
        static constexpr float btn_w{120.f * 2.f};
        static constexpr float btn_h{20.f * 2.f};
        battle_menu_gfx_button(const vec2f& pos, const battle_menu_choice& bmc)
            : _pos(pos), _bmc(bmc)
        {
            _tr.setAlign(ssvs::TextAlign::Center);
            _tr.eff<BTR::Tracking>(-3).in(_bmc.label()).mk("");
            _tr.setScale(vec2f(3.f, 3.f));

            vec2f shape_sz(btn_w, btn_h);
            _shape.setSize(shape_sz);
            // _shape.setOrigin(shape_sz / 2.f);
        }

        void update(game_app& app, battle_menu& bm, ft dt)
        {
            _shape.setPosition(_pos);
            _shape.setFillColor(is_hovered(app) ? sfc::Green : sfc::Red);

            auto lbtn_down(app.lb_down());

            if(is_hovered(app) && !bm._was_pressed && lbtn_down)
            {
                bm._was_pressed = true;
                _bmc.execute(bm);
            }

            if(!lbtn_down)
            {
                bm._was_pressed = false;
            }

            _tr.setPosition(_pos + vec2f(5.f, 5.f));

            ssvs::setOrigin(_tr, ssvs::getLocalCenter);
            _tr.setPosition(ssvs::getGlobalCenter(_shape));
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

            auto menu_h(200);
            auto menu_start_x(350.f);
            auto menu_start_y(game_constants::height - menu_h);
            auto menu_end_y(
                game_constants::height - battle_menu_gfx_button::btn_h);
            auto menu_end_x(game_constants::width - battle_menu_gfx_button::btn_w);
            auto offset(30.f);

            std::array<vec2f, 4> poss{
                vec2f{menu_start_x + offset, menu_start_y + offset}, // .
                vec2f{menu_end_x - offset, menu_start_y + offset},   // .
                vec2f{menu_start_x + offset, menu_end_y - offset},   // .
                vec2f{menu_end_x - offset, menu_end_y - offset},     // .
            };

            int i = 0;
            bs.for_choices([this, &i, &poss](const auto& cc)
                {
                    _buttons.emplace_back(poss[i], cc);
                    ++i;
                });
        }

        std::function<void()> _reb_fn;
        bool _must_reb{false};

    public:
        battle_menu_gfx_state() { _buttons.reserve(10); }

        void rebuild_from(const battle_menu& bm)
        {
            _reb_fn = [this, &bm]
            {
                rebuild_from(bm.current_screen());
            };
            _must_reb = true;
        }

        void update(game_app& app, battle_menu& bm, ft dt)
        {
            for(auto& b : _buttons) b.update(app, bm, dt);

            if(_must_reb)
            {
                _must_reb = false;
                _reb_fn();
            }
        }

        void draw(sf::RenderTarget& rt)
        {
            for(auto& b : _buttons) b.draw(rt);
        }
    };
}
GGJ16_NAMESPACE_END
