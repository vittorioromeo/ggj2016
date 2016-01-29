#include <iostream>

#include "base.hpp"
#include "game.hpp"
#include "assets.hpp"
#include "battle.hpp"

// Battle system process:
// 1. Every party member selects a ritual
// 2. Ritual minigames are executed
// 3.

GGJ16_NAMESPACE
{
    class battle_screen : public game_screen
    {
    private:
        using base_type = game_screen;

        battle_menu _menu;
        battle_menu_gfx_state _menu_gfx_state;

        battle_menu_screen* _m_main;
        battle_menu_screen* _m_ritual;
        battle_menu_screen* _m_item;

        battle _battle;

        void execute_ritual() {}

        void fill_ritual_menu()
        {
            auto& m(*_m_ritual);
            m.clear();

            m.emplace_choice("Ritual 0", [this](auto&)
                {
                    this->execute_ritual();
                });
            m.emplace_choice("Ritual 1", [this](auto&)
                {
                    this->execute_ritual();
                });
            m.emplace_choice("Ritual 2", [this](auto&)
                {
                    this->execute_ritual();
                });
            m.emplace_choice("Go back", [](auto& bm)
                {
                    bm.pop_screen();
                });
        }

        void fill_item_menu()
        {
            auto& m(*_m_item);
            m.clear();

            m.emplace_choice("Item 0", [this](auto&)
                {
                });
            m.emplace_choice("Item 1", [this](auto&)
                {
                });
            m.emplace_choice("Item 2", [this](auto&)
                {
                });
            m.emplace_choice("Go back", [](auto& bm)
                {
                    bm.pop_screen();
                });
        }

        void fill_main_menu()
        {
            auto& m(*_m_main);
            m.clear();

            m.emplace_choice("Perform ritual", [this](auto& bm)
                {
                    this->fill_ritual_menu();
                    bm.push_screen(*_m_ritual);
                });
            m.emplace_choice("Use item", [this](auto& bm)
                {
                    this->fill_item_menu();
                    bm.push_screen(*_m_item);
                });
        }

        void init_menu()
        {
            _m_main = &_menu.make_screen();
            _m_ritual = &_menu.make_screen();
            _m_item = &_menu.make_screen();

            fill_main_menu();

            _menu.on_change += [this]
            {
                _menu_gfx_state.rebuild_from(_menu);
            };

            _menu.push_screen(*_m_main);
        }

        void update_menu(ft dt) { _menu_gfx_state.update(app(), _menu, dt); }
        void draw_menu() { _menu_gfx_state.draw(app().window()); }

    public:
        battle_screen(game_app& app, const battle& battle) noexcept
            : base_type(app),
              _battle{battle}
        {
            init_menu();

            // Assume player starts
            if(true)
            {
            }
            else
            {
            }
        }

        void update(ft dt) override { update_menu(dt); }
        void draw() override { draw_menu(); }
    };
}
GGJ16_NAMESPACE_END

int main()
{
    using namespace ggj16;

    using game_app_runner = boilerplate::app_runner<game_app>;
    game_app_runner game{"ggj2016 temp", 320, 240};
    game_app& app(game.app());

    character_stats cs0;
    cs0.health() = 100;
    cs0.shield() = 40;
    cs0.power() = 10;
    cs0.defense() = 5;
    cs0.favour() = 10;
    battle_participant b0{cs0};
    cplayer_state ps;

    character_stats cs1;
    cs1.health() = 100;
    cs1.shield() = 40;
    cs1.power() = 10;
    cs1.defense() = 5;
    cs1.favour() = 10;
    battle_participant b1{cs1};

    battle b{ps, b0, b1};

    auto& test_battle(app.make_screen<battle_screen>(b));
    app.push_screen(test_battle);

    game.run();
    return 0;
}
