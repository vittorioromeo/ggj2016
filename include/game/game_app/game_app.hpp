#pragma once

#include "base/boilerplate.hpp"

GGJ16_NAMESPACE
{
    namespace game_constants
    {
        constexpr sz_t width{1024};
        constexpr sz_t height{768};
    }

    class game_app;

    class game_screen
    {
    private:
        game_app& _app;
        bool _pass_through{true};
        float _opacity_block{175};
        bool _dead{false};

    public:
        game_screen(game_app& app) noexcept : _app(app) {}

        inline virtual ~game_screen() {}

        virtual void update(ft) {}
        virtual void draw() {}

        game_app& app() { return _app; }

        void kill() { _dead = true; }
        const auto& dead() const noexcept { return _dead; }

        void set_pass_through(bool x) noexcept { _pass_through = x; }
        const auto& pass_through() const noexcept { return _pass_through; }

        void set_opacity_block(float x) noexcept { _opacity_block = x; }
        const auto& opacity_block() const noexcept { return _opacity_block; }
    };

    namespace impl
    {
        class game_screen_manager
        {
        private:
            using this_type = game_screen_manager;

        public:
            using game_screen_uptr = std::unique_ptr<game_screen>;
            using game_screen_storage = std::vector<game_screen_uptr>;
            using game_screen_stack = std::vector<game_screen*>;

        private:
            game_screen_storage _screen_storage;
            game_screen_stack _screen_stack;

        public:
            auto has_any_screen() const noexcept
            {
                return !_screen_stack.empty();
            }

            auto& current_screen() noexcept
            {
                VRM_CORE_ASSERT(has_any_screen());
                return *(_screen_stack.back());
            }

            const auto& current_screen() const noexcept
            {
                VRM_CORE_ASSERT(has_any_screen());
                return *(_screen_stack.back());
            }

            void update(ft dt) noexcept
            {
                ssvu::eraseRemoveIf(_screen_stack, [](const auto& s)
                    {
                        return s->dead();
                    });
                ssvu::eraseRemoveIf(_screen_storage, [](const auto& s)
                    {
                        return s->dead();
                    });

                if(!has_any_screen()) return;
                current_screen().update(dt);
            }
            void draw(sf::RenderTarget& rt) noexcept
            {
                if(!has_any_screen()) return;

                std::vector<std::function<void()>> to_draw;
                to_draw.emplace_back([this]
                    {
                        current_screen().draw();
                    });

                if(_screen_stack.size() > 1 && current_screen().pass_through())
                {
                    for(int i(_screen_stack.size() - 2); i >= 0; --i)
                    {
                        auto isc(_screen_stack[i]);

                        sf::RectangleShape block;
                        block.setPosition(vec2f(0, 0));
                        block.setSize(vec2f(
                            game_constants::width, game_constants::height));
                        block.setFillColor(sf::Color{0, 0, 0,
                            static_cast<sf::Uint8>(isc->opacity_block())});

                        to_draw.emplace_back([this, block, &rt]
                            {
                                rt.draw(block);
                            });

                        to_draw.emplace_back([this, isc]
                            {
                                isc->draw();
                            });
                    }
                }

                for(auto itr(std::rbegin(to_draw)); itr != std::rend(to_draw);
                    ++itr)
                {
                    (*itr)();
                }
            }

            template <typename T, typename... Ts>
            T& make_screen(Ts&&... xs)
            {
                VRM_CORE_STATIC_ASSERT_NM(std::is_base_of<game_screen, T>{});

                auto uptr(std::make_unique<T>(FWD(xs)...));
                _screen_storage.emplace_back(std::move(uptr));
                auto ptr(_screen_storage.back().get());
                return *vrmc::to_derived<T>(ptr);
            }

            void push_screen(game_screen& screen)
            {
                _screen_stack.emplace_back(&screen);
            }

            void pop_screen()
            {
                VRM_CORE_ASSERT_OP(_screen_stack.size(), >, 0);
                _screen_stack.pop_back();
            }
        };
    }

    class game_app : public boilerplate::app
    {
    public:
        float _shake{0};

    private:
        using this_type = game_app;

        impl::game_screen_manager _screen_manager;

        void init_loops()
        {
            auto orig_camera_pos = _camera.getCenter();
            state().onUpdate += [this, orig_camera_pos](ft dt)
            {
                _screen_manager.update(dt);

                if(_shake > 0)
                {
                    _shake -= dt * 0.6f;
                    _shake = std::abs(_shake);

                    _camera.setCenter(
                        orig_camera_pos +
                        vec2f{ssvu::getRndR(-_shake, _shake + 0.1f),
                            ssvu::getRndR(-_shake, _shake + 0.1f)});
                }
                else
                {
                    _camera.setCenter(orig_camera_pos);
                    _shake = 0;
                }

                _camera.update(dt);
            };

            state().onDraw += [this]
            {
                _camera.template apply<int>();
                _screen_manager.draw(window());
                _camera.unapply();
            };
        }

    public:
        inline game_app(ssvs::GameWindow& window) noexcept
            : boilerplate::app{window}
        {
            init_loops();
        }

        auto mp() const noexcept { return _camera.getMousePosition(); }

        auto lb_down() const noexcept
        {
            return window().getInputState()[ssvs::MBtn::Left];
        }

        auto rb_down() const noexcept
        {
            return window().getInputState()[ssvs::MBtn::Right];
        }

        template <typename T, typename... Ts>
        T& make_screen(Ts&&... xs)
        {
            return _screen_manager.template make_screen<T>(*this, FWD(xs)...);
        }

        void push_screen(game_screen& screen)
        {
            _screen_manager.push_screen(screen);
        }

        void pop_screen() { _screen_manager.pop_screen(); }
    };
}
GGJ16_NAMESPACE_END
