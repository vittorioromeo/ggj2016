#pragma once

#include "base/boilerplate.hpp"

GGJ16_NAMESPACE
{
    class game_app : public boilerplate::app
    {
    private:
        inline void update(ft dt) {}

        inline void draw()
        {
            camera().apply();
            camera().unapply();
        }

    public:
        inline game_app(ssvs::GameWindow& window) noexcept
            : boilerplate::app{window}
        {
            state().onUpdate += [this](ft dt)
            {
                update(dt);
            };

            state().onDraw += [this]
            {
                draw();
            };
        }
    };
}
GGJ16_NAMESPACE_END
