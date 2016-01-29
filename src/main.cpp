#include <iostream>
#include "base.hpp"
#include "game.hpp"

GGJ16_NAMESPACE
{
    class test_screen : public game_screen
    {
    private:
        using base_type = game_screen;

    public:
        test_screen(game_app& app) noexcept : base_type(app) {}

        void update(ft dt) override { std::cout << "Ping! " << dt << "\n"; }
    };
}
GGJ16_NAMESPACE_END

int main()
{
    using namespace ggj16;

    using game_app_runner = boilerplate::app_runner<game_app>;
    game_app_runner game{"ggj2016 temp", 320, 240};

    game_app& app(game.app());
    auto& my_test_screen(app.make_screen<test_screen>());
    app.push_screen(my_test_screen);

    game.run();
    return 0;
}
