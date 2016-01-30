#pragma once

#include <string>
#include <memory>
#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/Input/Input.hpp>

GGJ16_NAMESPACE
{
    namespace boilerplate
    {
        template <typename T>
        class app_runner
        {
        private:
            ssvs::GameWindow _window;
            std::unique_ptr<T> _app;

        public:
            inline app_runner(const std::string& title, ssvu::SizeT width,
                ssvu::SizeT height) noexcept
            {
                _window.setTitle(title);
                _window.setTimer<ssvs::TimerStatic>(0.5f, 0.5f);
                _window.setSize(width, height);
                _window.setFullscreen(false);
                _window.setFPSLimited(true);
                _window.setMaxFPS(200);
                _window.setPixelMult(1);

                _app = std::make_unique<T>(_window);
                _window.setGameState(_app->state());
            }

            inline auto& app() noexcept
            {
                VRM_CORE_ASSERT_OP(_app.get(), !=, nullptr);
                return *_app;
            }

            inline void run() noexcept { _window.run(); }
        };
    }
}
GGJ16_NAMESPACE_END
