#pragma once

#include <SSVStart/GameSystem/GameSystem.hpp>
#include <SSVStart/Camera/Camera.hpp>
#include <SSVStart/Input/Input.hpp>
#include "base/config/names.hpp"
#include "base/type_aliases.hpp"

GGJ16_NAMESPACE
{
    namespace boilerplate
    {
        class app
        {
        protected:
            ssvs::GameState _state;
            ssvs::Camera _camera;
            ssvs::GameWindow& _window;

        public:
            inline app(ssvs::GameWindow& window) noexcept
                : _camera{window, 1.f},
                  _window{window}
            {
            }

            inline void stop() noexcept { return _window.stop(); }

            template <typename... Ts>
            inline void render(Ts&&... xs) noexcept
            {
                _window.draw(FWD(xs)...);
            }

            inline auto& state() noexcept { return _state; }
            inline const auto& state() const noexcept { return _state; }
            inline auto& camera() noexcept { return _camera; }
            inline const auto& camera() const noexcept { return _camera; }
            inline auto& window() noexcept { return _window; }
            inline const auto& window() const noexcept { return _window; }
        };
    }
}
GGJ16_NAMESPACE_END
