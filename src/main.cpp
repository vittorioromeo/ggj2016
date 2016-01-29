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

    public:
        battle_screen(game_app& app) noexcept : base_type(app)
        {
            auto& ms(_menu.make_screen());
            auto& ms_inner(_menu.make_screen());

            ms.emplace_choice("hi", [](auto&)
                {
                });
            ms.emplace_choice("go fwd", [&ms_inner](auto& bm)
                {
                    bm.push_screen(ms_inner);
                });

            ms_inner.emplace_choice("bye", [](auto&)
                {
                });
            ms_inner.emplace_choice("ads", [](auto&)
                {
                });
            ms_inner.emplace_choice("go back", [](auto& bm)
                {
                    bm.pop_screen();
                });

            _menu.on_change += [this]
            {
                _menu_gfx_state.rebuild_from(_menu);
            };

            _menu.push_screen(ms);
        }

        void update(ft dt) override
        {
            _menu_gfx_state.update(app(), _menu, dt);
        }
        void draw() override { _menu_gfx_state.draw(app().window()); }
    };
}
GGJ16_NAMESPACE_END

int main()
{
    using namespace ggj16;

    using game_app_runner = boilerplate::app_runner<game_app>;
    game_app_runner game{"ggj2016 temp", 320, 240};

    game_app& app(game.app());
    auto& my_test_screen(app.make_screen<battle_screen>());
    app.push_screen(my_test_screen);

    game.run();
    return 0;
}
